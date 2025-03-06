/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#include "ScalarVisuallyGetAllGraphHandler.h"
#include <unordered_map>
#ifdef _WIN32
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#include "ScalarVisuallyServer.h"
#include "Util/ScalaryProtocolUtil.h"
using namespace Insight::Scalar;
using namespace Insight::Scalar::Protocol;

bool ScalarVisuallyGetAllGraphHandler::run(std::string_view data, std::string &resultStr) {
    resultStr = GetBasicResponse();
    std::unordered_map<std::string, std::vector<std::string> > res;
    res = ScalarVisuallyServer::Instance().GetAllGraphInfo();
    SetResponse(res, resultStr);
    return true;
}

void ScalarVisuallyGetAllGraphHandler::SetResponse(std::unordered_map<std::string,
        std::vector<std::string> > &graphInfoMap,
                                                   std::string &result) {
    document_t document = ParseJsonToStr(result);
    auto &allocator = document.GetAllocator();
    json data(rapidjson::kArrayType);
    for (auto &[tag, fileList]: graphInfoMap) {
        json graphInfo(rapidjson::kObjectType);
        AddJsonMember(graphInfo, "tag", tag, allocator);
        json fileListArray(rapidjson::kArrayType);
        for (auto &file: fileList) {
            json item(rapidjson::kObjectType);
            AddJsonMember(item, "path", file, allocator);
            AddJsonMember(item, "name", GetReadableFileName(file), allocator);
            fileListArray.PushBack(item, allocator);
        }
        AddJsonMember(graphInfo, "fileList", fileListArray, allocator);
        data.PushBack(graphInfo, allocator);
    }
    AddJsonMember(document["body"], "data", data, allocator);
    result = DumpJsonToStr(document);
}
