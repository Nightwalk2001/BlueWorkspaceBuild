/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#include "ScalarVisuallyServer.h"
#include <algorithm>
#include <future>
#include <numeric>
#include <sys/stat.h>

#include "Logger.h"
#include "FileManager/FileInfoManager.h"
#include "Util/FileUtil.h"

using namespace Insight::Scalar;

bool ScalarVisuallyServer::IsFileImported(std::string_view path) {
    auto fileInfo = fileManager_.GetFileInfo(path);
    if (fileInfo == nullptr) {
        return false;
    }
    if (!fileInfo->Imported()) {
        return false;
    }
    return true;
}

void ScalarVisuallyServer::OnFileDataUpdate(std::string &&dir, std::string &&fileName) {
    std::string filePath = dir + "/" + fileName;
    std::shared_ptr<FileInfo::File> file = fileManager_.GetFileInfo(filePath);
    if (file == nullptr) {
        return;
    }
    auto parser = GetFileParser(file->DataType());
    if (!parser) {
        return;
    }
    uint64_t offset = file->OffSet();
    bool empty = parser->ParserData(filePath, offset);
    file->SetOffSet(offset);

    if (!empty && file->Empty()) {
        ScalarVisuallyServer::Instance().OnFileCreate(std::move(dir), std::move(fileName));
        file->SetEmpty(false);
    }
}

bool ScalarVisuallyServer::IsFileSupported(std::string_view path) {
    return FileInfo::FileInfoManager::IsFileSupported(path);
}

std::shared_ptr<FileInfo::File> ScalarVisuallyServer::AddFile(const std::string &file)
{
    ParseDataType type = FileInfo::FileInfoManager::GetFileType(file);
    return fileManager_.AddFile(file, type);
}

void ScalarVisuallyServer::OnFileCreate(std::string &&dir, std::string &&fileName) {
    fileManager_.OnFileCreate(std::move(dir), std::move(fileName));
}

std::shared_ptr<Parser::FileParser> ScalarVisuallyServer::GetFileParser(ParseDataType type) {
    return Parser::ParserFactory::Instance().CreateFileParse(type);
}

void ScalarVisuallyServer::Reset() {
    fileManager_.Reset();
    graphManager_.Reset();
    fileWatcher_->Reset();
    incrementalTag_.clear();
    {
        std::unique_lock lock(parseMutex_);
        parseStateMap_.clear();
    }
    importedPaths_.clear();
}

void
ScalarVisuallyServer::UpdateGraph(const std::string &tag, const std::string &path, std::vector<ScalarPoint> &&data) {
    std::unique_lock lock(graphMutext_);
    graphManager_.UpdateGraphData(tag, path, std::move(data));
}

std::vector<std::string> ScalarVisuallyServer::GetGraphContainsFile(const std::string &tag) {
    std::unique_lock lock(graphMutext_);
    std::vector<std::string> res;
    auto graph = graphManager_.GetGraph(tag);
    if (graph == nullptr) {
        LOG(LogRank::Error) << "Graph not exit, tag=" << tag;
        return res;
    }
    return graph->GetDataFiles();
}

std::unordered_map<std::string, std::vector<std::string>> ScalarVisuallyServer::GetAllGraphInfo() {
    std::unique_lock lock(graphMutext_);
    return graphManager_.GetAllGraphInfo();
}

std::vector<DataView>
ScalarVisuallyServer::GetGraphData(const std::string &tag, const std::string &file, uint64_t left, uint64_t right,
                                   DataMode mode)
{
    std::unique_lock lock(graphMutext_);
    return graphManager_.GetGraphData(tag, file, left, right, mode);
}

void ScalarVisuallyServer::AddFileWatch(const std::vector<std::string> &fileList) {

    fileWatcher_->AddWatchPath(fileList);
}

std::unordered_map<std::string, std::set<std::string>> ScalarVisuallyServer::GetCreatedFileGroupByDir() {
    return fileManager_.GetCreatedFileGroupByDir();
}

std::string ScalarVisuallyServer::GetProjectName()
{
    static uint64_t projectId = 0;
    return std::to_string(projectId++);
}

bool ScalarVisuallyServer::AddParseTask(const std::string &projectName, std::vector<std::string> fileList)
{
    std::unique_lock lock(parseMutex_);
    std::vector<std::shared_ptr<FileInfo::File>> fileInfos;
    std::for_each(fileList.begin(), fileList.end(), [&fileInfos](const std::string &filePath) {
        if (ScalarVisuallyServer::Instance().IsFileImported(filePath)) {
            return;
        }
        auto fileInfo = ScalarVisuallyServer::Instance().AddFile(filePath);
        fileInfos.push_back(fileInfo);
    });
    // calculate total data size for calculate percent
    ParseState &s = parseStateMap_[projectName];
    s.dataSize_ = std::accumulate(fileInfos.begin(), fileInfos.end(), static_cast<uint64_t >(0),
                                  [](int acc, const std::shared_ptr<FileInfo::File> &fileInfo) {
                                      return acc + fs::file_size(fileInfo->FilePath());
                                  });
    s.percent_ = 0;
    s.finish_ = false;
    // add parsed thread
    std::for_each(fileInfos.begin(), fileInfos.end(),
                  [this, &projectName](const std::shared_ptr<FileInfo::File> &fileInfo) {
                      auto parser = this->GetFileParser(fileInfo->DataType());
                      if (parser == nullptr) {
                          return;
                      }
                      ParseState &state = parseStateMap_[projectName];
                      auto future = threadPool_->Submit([this, parser, fileInfo]() {
                          uint64_t offset = fileInfo->OffSet();
                          bool flag = parser->ParserData(fileInfo->FilePath(), offset);
                          fileInfo->SetOffSet(offset);
                          return flag;
                      });
                      state.parsedResults_.emplace(fileInfo->FilePath(), std::move(future));
                      state.fileInfos_.push_back(fileInfo);
                  });
    return true;
}

bool ScalarVisuallyServer::parseWatcher()
{
    for (;;) {
        // lock here, in case other thread delete parseState while use iter list
        {
            std::unique_lock lock(parseMutex_);
            for (auto &[projectName, state]: this->parseStateMap_) {
                // reduce
                if (state.finish_) {
                    continue;
                }
                // check future get value
                if (std::all_of(state.parsedResults_.begin(), state.parsedResults_.end(), [](auto &it) {
                    return it.second.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                })) {
                    state.finish_ = true;
                    state.percent_ = 100;
                    continue;
                }
                // update percent
                uint64_t parsedDataSize = 0;
                std::for_each(state.fileInfos_.begin(), state.fileInfos_.end(),
                              [&parsedDataSize](const auto &fileInfo) {
                                  parsedDataSize += fileInfo->OffSet();
                              });
                long double percent =
                        static_cast<long double>(parsedDataSize) / static_cast<long double>(state.dataSize_) * 100.0;
                state.percent_ = std::min(static_cast<uint32_t >(100), static_cast<uint32_t>(percent));

            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return true;
}

std::tuple<std::string, bool, uint32_t> ScalarVisuallyServer::GetProjectParseStatus(const std::string &projectName)
{
    std::unique_lock lock(parseMutex_);
    if (parseStateMap_.find(projectName) == parseStateMap_.end()) {
        return {projectName, true, 100};
    }
    ParseState &s = parseStateMap_[projectName];
    bool finish = s.finish_;
    uint32_t percent = s.percent_;
    if (s.finish_) {
        parseStateMap_.erase(projectName);
    }
    return {projectName, finish, percent};
}

bool ScalarVisuallyServer::IsIncremental(const std::string &filePath, const std::string &tag)
{
    // check whether new tag
    std::unique_lock lock(graphMutext_);
    auto graph = graphManager_.GetGraph(tag);
    if (graph == nullptr) {
        return true;
    }
    // check whether new file
    if (!graph->InnerFile(filePath)) {
        return true;
    }
    return false;
}

void ScalarVisuallyServer::AddIncremental(const std::string &filePath, const std::string &tag)
{
    std::unique_lock<std::mutex> lock(incrementalMutex_);
    incrementalTag_[tag].insert(filePath);
}

std::unordered_map<std::string, std::set<std::string>> ScalarVisuallyServer::GetIncremental()
{
    std::unique_lock lock(incrementalMutex_);
    constexpr uint64_t limits = 1000;
    uint64_t count = 0;
    std::unordered_map<std::string, std::set<std::string>> res;
    for (auto it = incrementalTag_.begin(); it != incrementalTag_.end();) {
        std::string tag = it->first;
        count += it->second.size();
        res.emplace(tag, std::move(it->second));
        it = incrementalTag_.erase(it);
        if (count > limits) {
            break;
        }
    }
    return res;
}

void ScalarVisuallyServer::UpdateSmoothingParam(const std::string &tag, const std::string &file,
                                                std::unique_ptr<Smoothing::SmoothingParamBase> param)
{
    return graphManager_.UpdateGraphSmoothingParam(tag, file, std::move(param));
}

std::shared_ptr<FileInfo::File> ScalarVisuallyServer::GetFileInfo(const std::string &file)
{
    return fileManager_.GetFileInfo(file);
}

void ScalarVisuallyServer::AddImportedPath(const std::string &path)
{
    importedPaths_.insert(path);
}

std::vector<std::string> ScalarVisuallyServer::GetFileHierarchy(const std::string &path)
{
    std::vector<std::string> res;
    for (auto &prefix: importedPaths_) {
        if (path.find(prefix) == 0) {
            res = FileUtil::GetFileHierarchy(prefix, path);
            return res;
        }
    }
    return {};
}

void ScalarVisuallyServer::AddVirtualFile(std::shared_ptr<FileInfo::File> fileInfo)
{
    if (fileInfo == nullptr) {
        return;
    }
    fileManager_.AddFile(fileInfo);
    const auto& tags = fileInfo->GetContainsTag();
    // 对每个表格添加文件信息
    std::for_each(tags.begin(), tags.end(), [&fileInfo](const std::string& tag) {
        auto graph = ScalarVisuallyServer::Instance().GetGraph(tag);
        if (graph == nullptr) {
            return;
        }
        if (!graph->InnerFile(fileInfo->FilePath())) {
            graph->UpdateData(fileInfo->FilePath(), {});
        }
    });
}

void ScalarVisuallyServer::DelVirtualFile(std::shared_ptr<FileInfo::File> file)
{
    if (file == nullptr) {
        return;
    }
    if (fileManager_.GetFileInfo(file->FilePath()) == nullptr) {
        return;
    }
    fileManager_.DelFileFromMap(file->FilePath());
    const auto& tags = file->GetContainsTag();
    std::for_each(tags.begin(), tags.end(), [&file](const std::string& tag) {
        auto graph = ScalarVisuallyServer::Instance().GetGraph(tag);
        if (graph == nullptr) {
            return;
        }
        graph->DelFile(file->FilePath());
    });
}

std::shared_ptr<Graph> ScalarVisuallyServer::GetGraph(const std::string& tag)
{
    return graphManager_.GetGraph(tag);
}

bool ScalarVisuallyServer::IsFileWatched(std::string &&path)
{
    return !(fileManager_.GetFileInfo(path) == nullptr);
}

void ScalarVisuallyServer::GetFileTags(std::set<std::string> &tags, std::string &path)
{
    return graphManager_.GetFileTags(path, tags);
}