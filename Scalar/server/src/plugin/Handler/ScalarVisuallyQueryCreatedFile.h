/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#ifndef SCALARVISUALLYQUERYNEWFILE_H
#define SCALARVISUALLYQUERYNEWFILE_H
#include <unordered_map>
#include <set>
#include "ApiHandler.h"

namespace Insight::Scalar {
using namespace Dic::Core;

class ScalarVisuallyQueryCreatedFile : public PostHandler {
public:
    bool run(std::string_view data, std::string &resultStr) override;

private:
    static void SetResponse(std::unordered_map<std::string, std::set<std::string>> &createdFileGroupByDir,
                            std::string &resultStr);
};
}

#endif //SCALARVISUALLYQUERYNEWFILE_H
