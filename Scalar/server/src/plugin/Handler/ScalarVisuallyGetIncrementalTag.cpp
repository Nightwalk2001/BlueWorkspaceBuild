/*
 * Copyright (c) , Huawei Technologies Co., Ltd. 2024-2024 .All rights reserved.
 */
#include "ScalarVisuallyGetIncrementalTag.h"
#include "ScalarVisuallyServer.h"
#include "Util/FileUtil.h"

using namespace Insight::Scalar;
using namespace Insight::Scalar::Protocol;

bool Insight::Scalar::ScalarVisuallyGetIncrementalTag::run(std::string_view data, std::string &result)
{
    result = GetBasicResponse();
    auto responseData = ScalarVisuallyServer::Instance().GetIncremental();
    SetResponse(std::move(responseData), result);
    return true;
}

void ScalarVisuallyGetIncrementalTag::SetResponse(std::unordered_map<std::string, std::set<std::string>> &&responseData,
                                                  std::string &result)
{
    document_t doc = ParseJsonToStr(result);
    auto &allocator = doc.GetAllocator();
    json_t data(rapidjson::kArrayType);
    for (auto &[tag, fileList]: responseData) {
        json_t tagInfo(rapidjson::kObjectType);
        AddJsonMember(tagInfo, "tag", tag, allocator);
        json_t fileListJson(rapidjson::kArrayType);
        std::for_each(fileList.begin(), fileList.end(), [&fileListJson, &allocator](const std::string &file) {
            json_t fileInfo(rapidjson::kObjectType);
            AddJsonMember(fileInfo, "name", GetReadableFileName(file), allocator);
            AddJsonMember(fileInfo, "path", file, allocator);
            AddJsonMember(fileInfo, "dirs", ScalarVisuallyServer::Instance().GetFileHierarchy(file), allocator);
            fileListJson.PushBack(fileInfo, allocator);
        });
        AddJsonMember(tagInfo, "fileList", fileListJson, allocator);
        data.PushBack(tagInfo, allocator);
    }
    AddJsonMember(doc["body"], "data", data, allocator);
    result = DumpJsonToStr(doc);
}
