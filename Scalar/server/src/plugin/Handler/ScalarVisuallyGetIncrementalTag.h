/*
 * Copyright (c) , Huawei Technologies Co., Ltd. 2024-2024 .All rights reserved.
 */
#ifndef ATT_SCALARVISUALLYGETINCREMENTALTAG_H
#define ATT_SCALARVISUALLYGETINCREMENTALTAG_H

#include <unordered_map>
#include "ApiHandler.h"
#include "Util/ScalaryProtocolUtil.h"

namespace Insight::Scalar {
using namespace Insight::Scalar::Protocol;
using namespace Dic::Core;

class ScalarVisuallyGetIncrementalTag : public PostHandler {
public:
    ScalarVisuallyGetIncrementalTag() = default;

    ~ScalarVisuallyGetIncrementalTag() override = default;

    bool run(std::string_view data, std::string &result) override;

private:
    static void SetResponse(std::unordered_map<std::string, std::set<std::string>> &&responseData, std::string &result);
};
}
#endif //ATT_SCALARVISUALLYGETINCREMENTALTAG_H
