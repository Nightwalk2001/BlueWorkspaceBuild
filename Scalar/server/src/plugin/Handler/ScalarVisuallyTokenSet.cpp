/*
* Copyright (c), Huawei Technologies Co., Ltd. 2025-2025.All rights reserved.
*/
#include "ScalarVisuallyTokenSet.h"
#include <algorithm>
#include "Logger.h"
#include "Util/ScalaryProtocolUtil.h"
#include "ScalarVisuallyServer.h"
using namespace Dic::Core;

bool ScalarVisuallyTokenSet::run(std::string_view data, std::string& resultStr)
{
    resultStr = Protocol::GetBasicResponse();
    std::string errMsg;
    TokenSetRequest request;
    if (auto errCode = ParseReqeustFromJson(data, request, errMsg); errCode != ErrCode::OK) {
        LOG(Insight::LogRank::Error) << errMsg;
        Protocol::SetResponseError(errCode, errMsg, resultStr);
    }
    TokenSetResponse response;
    std::for_each(request.tokenInfo_.begin(), request.tokenInfo_.end(), [&response, this](const auto& info) {
        TokenSetResponseItem item;
        std::tie(item.file_, item.globalBatchSize_, item.seqLength_) = info;
        item.tags_ = SetFileToken(item.file_, item.globalBatchSize_, item.seqLength_, response.errMsg_);
        response.items_.emplace_back(std::move(item));
    });
    SetResponse(std::move(response), resultStr);
    return true;
}

bool ScalarVisuallyTokenSet::CheckParamValid(const Insight::Scalar::document_t& request)
{
    if (! request.HasMember("params")) {
        return false;
    }
    const auto& param = request["params"];
    if (!param.IsArray()) {
        return false;
    }
    return std::all_of(param.Begin(), param.End(), [](const json_t& item) {
        if (!item.IsObject()) {
            return false;
        }
        if (!item.HasMember("file") || !item["file"].IsString()) {
            return false;
        }
        if (!item.HasMember("globalBatchSize") || !item["globalBatchSize"].IsNumber()) {
            return false;
        }
        if (!item.HasMember("seqLength") || !item["seqLength"].IsNumber()) {
            return false;
        }
        return true;
    });
}


ErrCode ScalarVisuallyTokenSet::ParseReqeustFromJson(std::string_view data, TokenSetRequest& request,
                                                     std::string& errMsg)
{
    std::string parseErr;
    std::optional<document_t> document = Protocol::TryParseJson<rapidjson::ParseFlag::kParseDefaultFlags>(
        data, parseErr);
    if (!document.has_value()) {
        errMsg = "Invalid request json, error:" + parseErr;
        return ErrCode::INVALID_REQUEST_JSON;
    }
    if (!CheckParamValid(document.value())) {
        errMsg = "Invalid request param";
        return ErrCode::REQUEST_INVALID_PARAM;
    }
    auto& requestJson = document.value();
    auto& params = requestJson["params"];
    std::for_each(params.Begin(), params.End(), [&request](const json_t& param) {
        std::string file = param["file"].GetString();
        double globalBatchSize = param["globalBatchSize"].GetDouble();
        double seqLength = param["seqLength"].GetDouble();
        request.tokenInfo_.emplace_back(file, globalBatchSize, seqLength);
    });
    return ErrCode::OK;
}

std::set<std::string> ScalarVisuallyTokenSet::SetFileToken(const std::string& file, double globalBatchSize,
                                                           double seqLength, std::string& errMsg)
{
    auto fileInfo = ScalarVisuallyServer::Instance().GetFileInfo(file);
    if (fileInfo == nullptr) {
        errMsg.append("file " + file + "Not exist \n");
        return {};
    }
    if (fileInfo->Type() == FileInfo::FileType::VISUAL) {
        errMsg.append("visual file can't config token");
        return {};
    }
    fileInfo->SetGlobalBatchSize(globalBatchSize);
    fileInfo->SetSeqLength(seqLength);
    return fileInfo->GetContainsTag();
}

void ScalarVisuallyTokenSet::SetResponse(TokenSetResponse&& response, std::string& resultStr)
{
    document_t result = ParseJsonToStr(resultStr);
    auto& allocator = result.GetAllocator();
    json_t data(rapidjson::kArrayType);
    for (auto& item : response.items_) {
        json_t itemJson(rapidjson::kObjectType);
        AddJsonMember(itemJson, "file", item.file_, allocator);
        AddJsonMember(itemJson, "globalBatchSize", item.globalBatchSize_, allocator);
        AddJsonMember(itemJson, "seqLength", item.seqLength_, allocator);
        AddJsonMember(itemJson, "tag", item.tags_, allocator);
        data.PushBack(itemJson, allocator);
    }
    AddJsonMember(result["body"], "data", data, allocator);
    resultStr = DumpJsonToStr(result);
}
