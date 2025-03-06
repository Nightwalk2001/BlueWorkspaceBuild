/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#include "ScalarVisuallyQueryCreatedFile.h"
#include "Logger.h"
#include "defs/ConceptDefs.h"
#include "ScalarVisuallyServer.h"
#include "Util/ScalaryProtocolUtil.h"

using namespace Insight::Scalar;
using namespace Insight::Scalar::Protocol;

bool ScalarVisuallyQueryCreatedFile::run(std::string_view data, std::string &resultStr) {
    resultStr = GetBasicResponse();
    std::unordered_map<std::string, std::set<std::string>>
            createFileGroupByDir = ScalarVisuallyServer::Instance().GetCreatedFileGroupByDir();
    SetResponse(createFileGroupByDir, resultStr);
    return true;
}

void ScalarVisuallyQueryCreatedFile::SetResponse(std::unordered_map<std::string,
        std::set<std::string>> &createdFileGroupByDir,
                                                 std::string &resultStr) {
    document_t result = ParseJsonToStr(resultStr);
    auto &allocator = result.GetAllocator();
    json data(rapidjson::kArrayType);
    for (auto &[dir, fileList]: createdFileGroupByDir) {
        json createFile(rapidjson::kObjectType);
        AddJsonMember(createFile, "dir", dir, allocator);
        AddJsonMember(createFile, "fileList", fileList, allocator);
        data.PushBack(createFile, allocator);
    }
    AddJsonMember(result["body"], "data", data, allocator);
    resultStr = DumpJsonToStr(result);
}
