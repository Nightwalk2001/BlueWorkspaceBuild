/*
* Copyright (c), Huawei Technologies Co., Ltd. 2025-2025.All rights reserved.
*/
#include "ScalarVisuallyFileMergeHandler.h"
#include <random>
#include "Logger.h"
#include "ScalarVisuallyServer.h"
#include "FileManager/VirtualFile.h"
using namespace Dic::Core;
using namespace Insight::Scalar::Protocol;
using namespace Insight::Scalar::FileInfo;
bool ScalarVisuallyFileMergeHandler::run(std::string_view data, std::string& result)
{
    result = ::Protocol::GetBasicResponse();
    std::string errMsg;
    FileMergeRequest request;
    if (auto errCode = ParseRequestFromJson(data, request, errMsg); errCode != ErrCode::OK) {
        LOG(Insight::LogRank::Error) << errMsg;
        SetResponseError(errCode, errMsg, result);
        return false;
    }
    auto InValidFile = [&request](const std::string& file) {
        auto fileInfo = ScalarVisuallyServer::Instance().GetFileInfo(file);
        if (fileInfo == nullptr) {
            return true;
        }
        if (fileInfo->Type() == FileInfo::FileType::VISUAL) {
            return true;
        }
        return false;
    };
    if (std::any_of(request.files_.begin(), request.files_.end(), InValidFile)) {
        SetResponseError(ErrCode::REQUEST_INVALID_PARAM, "Invalid File", result);
        return false;
    }
    FileMergeResponse response;
    response.action_ = request.merge_ ? "merge" : "unset";
    response.name_ = request.name_;
    if (request.merge_) {
        auto fileInfo = FileMerge(std::move(request.files_), request.name_);
        if (fileInfo == nullptr) {
            SetResponseError(ErrCode::REQUEST_INVALID_PARAM, "Merge failed", result);
            return true;
        }
        response.tag_ = fileInfo->GetContainsTag();
        response.file_ = fileInfo->FilePath();
        response.subFiles_ = reinterpret_cast<VirtualFile*>(fileInfo.get())->GetSubFiles();
    } else {
        auto fileInfo = ScalarVisuallyServer::Instance().GetFileInfo(request.name_);
        if (fileInfo == nullptr || fileInfo->Type() != FileType::VISUAL) {
            SetResponseError(ErrCode::REQUEST_INVALID_PARAM, "Unmerge failed", result);
            return true;
        }
        response.tag_ = fileInfo->GetContainsTag();
        response.file_ = fileInfo->FilePath();
        response.subFiles_ = {};
        FileUnMerge(request.name_);
    }
    SetResponse(std::move(response), result);
    return true;
}

ErrCode ScalarVisuallyFileMergeHandler::ParseRequestFromJson(std::string_view data, FileMergeRequest& request,
                                                             std::string& errMsg)
{
    std::string parseErr;
    std::optional<document_t> document = ParseJsonToStr(data);
    if (!document.has_value()) {
        errMsg = "Invalid request json," + parseErr;
        return ErrCode::INVALID_REQUEST_JSON;
    }
    if (!CheckParamValid(document.value())) {
        errMsg = "Invalid request param";
        return ErrCode::REQUEST_INVALID_PARAM;
    }
    auto& param = document.value();
    std::string action = param["action"].GetString();
    request.merge_ = (action == "merge");
    request.name_ = param["name"].GetString();
    std::transform(param["fileList"].Begin(), param["fileList"].End(), std::back_inserter(request.files_),
                   [](const auto& item) {
                       return item.GetString();
                   });
    return ErrCode::OK;
}

bool ScalarVisuallyFileMergeHandler::CheckParamValid(document_t& request)
{
    if (!request.HasMember("action") || !request["action"].IsString()) {
        return false;
    }
    if (!request.HasMember("name") || !request["name"].IsString()) {
        return false;
    }
    if (!request.HasMember("fileList") || !request["fileList"].IsArray()) {
        return false;
    }
    return std::all_of(request["fileList"].Begin(), request["fileList"].End(), [](const auto& item) {
        return item.IsString();
    });
}

void ScalarVisuallyFileMergeHandler::SetResponse(FileMergeResponse&& response, std::string& resultStr)
{
    document_t documet = ParseJsonToStr(resultStr);
    auto& allocator = documet.GetAllocator();
    json_t data(rapidjson::kObjectType);
    AddJsonMember(data, "file", response.file_, allocator);
    AddJsonMember(data, "tags", response.tag_, allocator);
    AddJsonMember(data, "fileList", response.subFiles_, allocator);
    AddJsonMember(data, "action", response.action_, allocator);
    AddJsonMember(documet["body"], "data", data, allocator);
    resultStr = DumpJsonToStr(documet);
}

std::shared_ptr<FileInfo::File> ScalarVisuallyFileMergeHandler::FileMerge(std::vector<std::string>&& subFiles, const std::string& name)
{
    std::string fileName = !name.empty() ? name : GeneratorFileName();
    auto fileInfo = std::make_shared<VirtualFile>(name);
    fileInfo->AddSubFiles(subFiles);
    ScalarVisuallyServer::Instance().AddVirtualFile(fileInfo);
    return fileInfo;
}

std::string ScalarVisuallyFileMergeHandler::GeneratorFileName()
{
    static const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<size_t> dist(0, charset.size() - 1);
    constexpr int randomStrLen = 8;
    std::string result;
    for (size_t i = 0; i < randomStrLen; ++i) {
        result += charset[dist(rng)];
    }
    // 获取当前时间戳
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) % 1000;

    // 格式化时间戳
    std::stringstream ss;
    ss << now_ms.count();

    // 拼接文件名
    return result + "_" + ss.str();
}

void ScalarVisuallyFileMergeHandler::FileUnMerge(const std::string& file)
{
    if (file.empty()) {
        return;
    }
    auto fileInfo = ScalarVisuallyServer::Instance().GetFileInfo(file);
    if (fileInfo == nullptr) {
        return;
    }
    if (fileInfo->Type() != FileType::VISUAL) {
        return;
    }
    ScalarVisuallyServer::Instance().DelVirtualFile(fileInfo);
}
