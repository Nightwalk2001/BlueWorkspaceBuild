/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#ifndef BOARD_PLUGINS_SCALARVISUALLY_SRC_PLUGIN_HANDLER_SCALARVISUALLYGETSCALARDATAHANDLER_H_
#define BOARD_PLUGINS_SCALARVISUALLY_SRC_PLUGIN_HANDLER_SCALARVISUALLYGETSCALARDATAHANDLER_H_

#include <iosfwd>
#include <iosfwd>
#include <vector>
#include <vector>

#include "ApiHandler.h"
#include "ScalarVisuallyServer.h"
#include "Util/ScalaryProtocolUtil.h"
#include "GraphManager/GraphManager.h"

namespace Insight::Scalar {
using namespace Dic::Core;
using namespace Scalar::Protocol;
using namespace Scalar::GraphOp;
struct ScalarResponse {
    std::string tag_;
    std::string file_;
    std::vector<LineData> lines_;
    std::vector<std::tuple<uint64_t, double, std::string>> dateIndex_;
};

class ScalarVisuallyGetScalarDataHandler : public PostHandler {
public:
    bool run(std::string_view data, std::string &resultStr) override;

    static bool CheckParamValid(const document_t &request);

private:
    std::vector<GraphLine> BuildLines(const std::unordered_map<LineType, std::shared_ptr<LineOp>>& opMap);

    static void SetResponse(std::vector<ScalarResponse> &&responseData, std::string &resultStr);

    std::vector<std::shared_ptr<LineOp>> GetLineOps(LineType type,  const std::unordered_map<LineType, std::shared_ptr<LineOp>>& opMap);

    static std::unordered_map<LineType, std::shared_ptr<LineOp>> BuildLineOP(const SingleGraphReqInfo& graphReq);

    static std::vector<std::tuple<uint64_t, double, std::string>>
    BuildDateIndex(std::vector<ScalarPoint>::iterator begin, std::vector<ScalarPoint>::iterator end);

    ScalarResponse ProcessSingleGraphReq(const SingleGraphReqInfo& graphReq);

    static inline ErrCode
    ParseRequestFromJson(std::string_view data, GetScalarDataRequest &request, std::string &errMsg);
};
}

#endif //BOARD_PLUGINS_SCALARVISUALLY_SRC_PLUGIN_HANDLER_SCALARVISUALLYGETSCALARDATAHANDLER_H_
