/*
 * Copyright (c) , Huawei Technologies Co., Ltd. 2024-2024 .All rights reserved.
 */
#ifndef ATT_SCALARVISUALLYGETPARSESTATE_H
#define ATT_SCALARVISUALLYGETPARSESTATE_H

#include <tuple>
#include <vector>
#include "ApiHandler.h"
#include "Util/ScalaryProtocolUtil.h"

namespace Insight::Scalar {
using namespace Dic::Core;
using namespace Insight::Scalar::Protocol;

class ScalarVisuallyGetParseState : public PostHandler {
    using ParseState = std::tuple<std::string, bool, uint32_t>;
public:
    ScalarVisuallyGetParseState() = default;

    ~ScalarVisuallyGetParseState() override = default;

    bool run(std::string_view data, std::string &result) override;

    static bool CheckParamValid(const document_t &request);

private:
    static void SetResponse(std::vector<ParseState> &response, std::string &resultStr);

    static inline ErrCode
    ParseRequestFromJson(std::string_view data, std::vector<std::string> &projectNameList, std::string &errMsg);
};
}


#endif //ATT_SCALARVISUALLYGETPARSESTATE_H
