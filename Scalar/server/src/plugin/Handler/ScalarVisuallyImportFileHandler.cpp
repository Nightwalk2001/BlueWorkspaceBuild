/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#include "ScalarVisuallyImportFileHandler.h"
#include <algorithm>
#include <queue>
#include <tuple>

#ifdef _WIN32
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#include "ScalarVisuallyServer.h"
#include "Logger.h"
#include "Util/FileUtil.h"
#include "Util/ScalaryProtocolUtil.h"
using namespace Insight::Scalar;
using namespace Insight;

bool ScalarVisuallyImportFileHandler::run(std::string_view data, std::string &resultStr) {
    resultStr = GetBasicResponse();
    std::string errMsg;
    ImportFileRequest request;
    if (auto errCode = ParseReqeustFromJson(data, request, errMsg); errCode != ErrCode::OK) {
        LOG(LogRank::Error) << errMsg;
        SetResponseError(errCode, errMsg, resultStr);
        return false;
    }
    if (!request.append_) {
        ScalarVisuallyServer::Instance().Reset();
    }
    auto &pathList = request.pathList_;
    std::string projectName = ScalarVisuallyServer::GetProjectName();
    std::for_each(pathList.begin(), pathList.end(), [](const std::string &filePath) {
        if (fs::is_directory(filePath)) {
            ScalarVisuallyServer::Instance().AddImportedPath(filePath);
        } else {
            ScalarVisuallyServer::Instance().AddImportedPath(fs::path(filePath).parent_path().string());
        }
    });
    // get all file which need to be import
    std::vector<std::string> importFiles = GetImportFiles(pathList);
    ScalarVisuallyServer::Instance().AddParseTask(projectName, importFiles);
    SetResponse(projectName, resultStr);
    return true;
}

bool ScalarVisuallyImportFileHandler::PathInvalid(std::string_view path) {
    if (path.empty()) {
        LOG(LogRank::Info) << "path is empty";
        return true;
    }
    if (!fs::is_regular_file(path) && !fs::is_directory(path)) {
        LOG(LogRank::Info) << "path is not a normal file or dir, path=" << path;
        return true;
    }
    return false;
}

std::vector<std::string> ScalarVisuallyImportFileHandler::GetImportFiles(std::vector<std::string> &pathList) {
    std::vector<std::string> res;
    for (const auto &path: pathList) {
        try {
            if (!fs::is_directory(path) && ScalarVisuallyServer::IsFileSupported(path)) {
                res.emplace_back(path);
            }
            constexpr uint32_t MAX_DEPTH = 7;
            RecursiveScanFolder(path, res, MAX_DEPTH);
        } catch (const fs::filesystem_error &e) {
            LOG(LogRank::Error) << "Cause filesystem error when import, e =" << e.what();
            continue;
        }
    }
    return res;
}

void ScalarVisuallyImportFileHandler::SetResponse(const std::string &projectName, std::string &resultStr)
{
    document_t document = ParseJsonToStr(resultStr);
    auto &allocator = document.GetAllocator();
    json &body = document["body"];
    AddJsonMember(body, "projectName", projectName, allocator);
    resultStr = DumpJsonToStr(document);
}

void ScalarVisuallyImportFileHandler::AddFileWatch(const std::string &path) {
    if (path.empty()) {
        return;
    }
    ScalarVisuallyServer::Instance().AddFileWatch({path});
}

bool ScalarVisuallyImportFileHandler::CheckParamValid(const document_t &request) {
    if (!request.HasMember("append") || !request["append"].IsBool()) {
        return false;
    }
    if (!request.HasMember("pathList") || !request["pathList"].IsArray()) {
        return false;
    }
    return std::all_of(request["pathList"].Begin(), request["pathList"].End(), [](const json &item) {
        return item.IsString();
    });
}

ErrCode ScalarVisuallyImportFileHandler::ParseReqeustFromJson(std::string_view data,
                                                              Insight::Scalar::ImportFileRequest &request,
                                                              std::string &errMsg) {
    std::string parseErr;
    std::optional<document_t> document = TryParseJson<rapidjson::ParseFlag::kParseDefaultFlags>(data, parseErr);
    if (!document.has_value()) {
        errMsg = "Invalid request json, error:" + parseErr;
        return ErrCode::INVALID_REQUEST_JSON;
    }
    if (!CheckParamValid(document.value())) {
        errMsg = "Invalid request param";
        return ErrCode::REQUEST_INVALID_PARAM;
    }
    auto &requestJson = document.value();
    request.append_ = requestJson["append"].GetBool();
    std::transform(requestJson["pathList"].Begin(),
                   requestJson["pathList"].End(),
                   std::back_inserter(request.pathList_),
                   [](const auto &item) {
                       return std::string(item.GetString());
                   });
    return ErrCode::OK;
}

void ScalarVisuallyImportFileHandler::RecursiveScanFolder(const std::string &path,
                                                          std::vector<std::string> &fileList,
                                                          int maxDepth) {
    if (path.empty() || maxDepth < 0) {
        return;
    }
    if (!fs::exists(path) || !fs::is_directory(path)) {
        return;
    }
    std::queue<std::tuple<std::string, int> > searchQueue;
    searchQueue.push({path, 0});
    while (!searchQueue.empty()) {
        auto [curPath, curDepth] = searchQueue.front();
        searchQueue.pop();
        if (curDepth == maxDepth) {
            continue;
        }
        if (auto per = fs::status(curPath).permissions(); (per & fs::perms::owner_read) == fs::perms::none) {
            LOG(LogRank::Error) << "Cur path has no read permission";
            continue;
        }
        for (const auto &entry: fs::directory_iterator(curPath)) {
            if (fs::is_directory(entry)) {
                searchQueue.emplace(entry.path().string(), curDepth + 1);
                continue;
            }
            if (fs::is_regular_file(entry) && ScalarVisuallyServer::IsFileSupported(entry.path().string())) {
                fileList.emplace_back(entry.path().string());
            }
        }
    }
}
