/*
 * Copyright (c) , Huawei Technologies Co., Ltd. 2024-2024 .All rights reserved.
 */
#include "ScalarVisuallyGetParseState.h"
#include "ScalarVisuallyServer.h"
#include <algorithm>

using namespace Insight::Scalar;
using namespace Insight::Scalar::Protocol;

bool Insight::Scalar::ScalarVisuallyGetParseState::run(std::string_view data, std::string &result)
{
    result = GetBasicResponse();
    std::string errMsg;
    std::vector<std::string> projectNameList;
    if (auto errCode = ParseRequestFromJson(data, projectNameList, errMsg); errCode != ErrCode::OK) {
        SetResponseError(errCode, errMsg, result);
        return false;
    }
    std::vector<ParseState> parsedState;
    std::transform(projectNameList.begin(), projectNameList.end(), std::back_inserter(parsedState),
                   [](const std::string &projectName) {
                       return ScalarVisuallyServer::Instance().GetProjectParseStatus(projectName);
                   });
    SetResponse(parsedState, result);
    return true;
}

bool Insight::Scalar::ScalarVisuallyGetParseState::CheckParamValid(const Insight::Scalar::document_t &request)
{
    if (!request.IsObject()) {
        return false;
    }
    if (!request.HasMember("projectNameLists") || !request["projectNameLists"].IsArray()) {
        return false;
    }
    return std::all_of(request["projectNameLists"].Begin(), request["projectNameLists"].End(), [](const json_t &item) {
        return item.IsString();
    });
}

ErrCode
ScalarVisuallyGetParseState::ParseRequestFromJson(std::string_view data, std::vector<std::string> &projectNameList,
                                                  std::string &errMsg)
{
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
    json_t &nameList = document.value()["projectNameLists"];
    std::transform(nameList.Begin(), nameList.End(), std::back_inserter(projectNameList), [](const auto &name) {
        return name.GetString();
    });
    return ErrCode::OK;
}

void ScalarVisuallyGetParseState::SetResponse(std::vector<ParseState> &response, std::string &resultStr)
{
    document_t doc = ParseJsonToStr(resultStr);
    auto &allocator = doc.GetAllocator();
    json_t stateList(rapidjson::kArrayType);
    std::for_each(response.begin(), response.end(), [&stateList, & allocator](const ParseState &state) {
        json_t stateJson(rapidjson::kObjectType);
        AddJsonMember(stateJson, "projectName", std::get<0>(state), allocator);
        AddJsonMember(stateJson, "finish", std::get<1>(state), allocator);
        AddJsonMember(stateJson, "percent", std::get<2>(state), allocator);
        stateList.PushBack(stateJson, allocator);
    });
    AddJsonMember(doc["body"], "stateList", stateList, allocator);
    resultStr = DumpJsonToStr(doc);
}
