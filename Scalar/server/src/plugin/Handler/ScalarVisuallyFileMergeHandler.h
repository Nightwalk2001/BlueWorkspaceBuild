/*
* Copyright (c), Huawei Technologies Co., Ltd. 2025-2025.All rights reserved.
*/

#ifndef SCALARVISUALLYFILEMERGEHANDLER_H
#define SCALARVISUALLYFILEMERGEHANDLER_H
#include <set>
#include <ProtocolUtil.h>
#include <Util/ScalaryProtocolUtil.h>

#include "ApiHandler.h"
#include "FileManager/File.h"
using namespace Insight::Scalar;
namespace Dic::Core {

struct FileMergeRequest
{
    bool merge_;  // true-合并， false-取消合并
    std::string name_;
    std::vector<std::string> files_;
};

struct FileMergeResponse
{
    std::string file_;
    std::string action_;
    std::string name_;
    std::set<std::string> subFiles_;
    std::set<std::string> tag_;
};
class ScalarVisuallyFileMergeHandler: public PostHandler {
public:
    ScalarVisuallyFileMergeHandler() = default;
    ~ScalarVisuallyFileMergeHandler() override = default;

    bool run(std::string_view data, std::string& result) override;
    static bool CheckParamValid(document_t& request);
private:
    static void SetResponse(FileMergeResponse&& response, std::string& resultStr);

    static  ErrCode ParseRequestFromJson(std::string_view data, FileMergeRequest& request ,std::string& errMsg);

    static std::string GeneratorFileName();

    std::shared_ptr<FileInfo::File> FileMerge(std::vector<std::string>&& subFiles, const std::string& name);
    void FileUnMerge(const std::string& file);
};
}




#endif //SCALARVISUALLYFILEMERGEHANDLER_H
