/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#ifndef BOARD_SCALARVISUALLYIMPORTFILEHANDLER_H
#define BOARD_SCALARVISUALLYIMPORTFILEHANDLER_H

#include <set>
#include <vector>
#include "ApiHandler.h"
#include "Util/ScalaryProtocolUtil.h"

using namespace Dic::Core;
namespace Insight::Scalar {
using namespace Insight::Scalar::Protocol;
struct ImportFileRequest {
    std::vector<std::string> pathList_;
    bool append_{false};
};

class ScalarVisuallyImportFileHandler : public PostHandler {
public:
    bool run(std::string_view data, std::string &resultStr) override;

    static bool CheckParamValid(const document_t &request);

private:

    static ErrCode ParseReqeustFromJson(std::string_view data, ImportFileRequest &request, std::string &errMsg);

    static bool PathInvalid(std::string_view);

    static std::vector<std::string> GetImportFiles(std::vector<std::string> &pathList);

    static void SetResponse(const std::string &projectName, std::string &resultStr);

    static void AddFileWatch(const std::string &path);

    static void RecursiveScanFolder(const std::string &path,
                                    std::vector<std::string> &fileList,
                                    int maxDepth);
};
}

#endif //BOARD_SCALARVISUALLYIMPORTFILEHANDLER_H
