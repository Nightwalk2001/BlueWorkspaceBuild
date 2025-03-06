/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#ifndef BOARD_SCALARVISUALLYSERVER_H
#define BOARD_SCALARVISUALLYSERVER_H

#include <vector>
#include <string>
#include <thread>
#include <algorithm>
#include <future>
#include <unordered_map>
#include <string_view>
#include "defs/ConceptDefs.h"
#include "FileManager/FileInfoManager.h"
#include "GraphManager/GraphManager.h"
#include "FileManager/FileWatcherFactory.h"
#include "ParserFactory.h"
#include "Logger.h"
#include "Util/ScalaryProtocolUtil.h"
#include "Util/ThreadPoolUtil.h"

using namespace Insight;
namespace Insight::Scalar {
using namespace GraphOp;
using namespace Protocol;
using namespace Dic::Core;

struct ParseState {
    std::unordered_map<std::string, std::future<bool>> parsedResults_;
    std::vector<std::shared_ptr<FileInfo::File>> fileInfos_;
    uint64_t dataSize_{0};
    uint32_t percent_{0};
    bool finish_{false};
};

class ScalarVisuallyServer {
public:
    static ScalarVisuallyServer &Instance() {
        static ScalarVisuallyServer instance;
        return instance;
    }

    ~ScalarVisuallyServer() = default;

    void AddImportedPath(const std::string &path);

    std::vector<std::string> GetFileHierarchy(const std::string &path);
    // 文件增删改查
    std::shared_ptr<FileInfo::File> AddFile(const std::string &file);
    void AddVirtualFile(std::shared_ptr<FileInfo::File> fileInfo);
    bool IsFileImported(std::string_view path);
    void AddFileWatch(const std::vector<std::string> &fileList);
    bool IsFileWatched(std::string &&path);
    static bool IsFileSupported(std::string_view path);
    void GetFileTags(std::set<std::string> &tags, std::string &path);
    std::unordered_map<std::string, std::set<std::string> > GetCreatedFileGroupByDir();
    std::shared_ptr<FileInfo::File> GetFileInfo(const std::string &file);
    void DelVirtualFile(std::shared_ptr<FileInfo::File> file);

    // Parse file func
    bool AddParseTask(const std::string &projectName, std::vector<std::string> fileList);

    bool parseWatcher();

    std::tuple<std::string, bool, uint32_t> GetProjectParseStatus(const std::string &projectName);

    bool IsIncremental(const std::string &filePath, const std::string &tag);

    void AddIncremental(const std::string &filePath, const std::string &tag);

    std::unordered_map<std::string, std::set<std::string>> GetIncremental();

    // 文件更新
    void OnFileDataUpdate(std::string &&dir, std::string &&fileName);
    void OnFileCreate(std::string &&dir, std::string &&fileName);
    std::shared_ptr<Parser::FileParser> GetFileParser(ParseDataType type);

    // 表格数据
    std::shared_ptr<Graph> GetGraph(const std::string& tag);
    std::vector<std::string> GetGraphContainsFile(const std::string &tag);

    std::vector<DataView> GetGraphData(const std::string &tag, const std::string &file, uint64_t left, uint64_t right,
                                       DataMode mode);

    void UpdateSmoothingParam(const std::string &tag, const std::string &file,
                              std::unique_ptr<Smoothing::SmoothingParamBase> param);

    std::unordered_map<std::string, std::vector<std::string> > GetAllGraphInfo();
    void UpdateGraph(const std::string &tag, const std::string &path, std::vector<ScalarPoint> &&data);


    static std::string GetProjectName();
    void Reset();

private:
    ScalarVisuallyServer() noexcept
    {
        fileWatcher_ = FileWatch::FileWatcherFactory::GetFileWatcher();
        if (!fileWatcher_) {
            LOG(LogRank::Warning) << "This platform not support file realtime watch";
            return;
        }
        fileWatcher_->Init();
        threadPool_ = std::make_shared<ThreadExecutor>(
                std::min(std::thread::hardware_concurrency(), static_cast<unsigned >(16)));

        threadPool_->Submit([this] { return parseWatcher(); });
    };

    Scalar::FileInfo::FileInfoManager fileManager_{};
    GraphManager graphManager_{};
    std::unique_ptr<Scalar::FileWatch::FileWatcher> fileWatcher_{nullptr};
    std::shared_ptr<ThreadExecutor> threadPool_{nullptr};
    std::unordered_map<std::string, ParseState> parseStateMap_{};
    std::unordered_map<std::string, std::set<std::string>> incrementalTag_{}; // tag - [filePath]
    std::mutex incrementalMutex_{};
    std::mutex parseMutex_{};
    std::mutex graphMutext_{};
    std::set<std::string> importedPaths_{};
};
}

#endif //BOARD_SCALARVISUALLYSERVER_H
