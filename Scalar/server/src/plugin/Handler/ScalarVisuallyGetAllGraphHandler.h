/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#ifndef BOARD_PLUGINS_SCALARVISUALLY_SRC_PLUGIN_HANDLER_SCALARVISUALLYGETALLGRAPHHANDLER_H_
#define BOARD_PLUGINS_SCALARVISUALLY_SRC_PLUGIN_HANDLER_SCALARVISUALLYGETALLGRAPHHANDLER_H_

#include <unordered_map>
#include <string>
#include <vector>
#include "ApiHandler.h"
using namespace Dic::Core;
namespace Insight::Scalar {
class ScalarVisuallyGetAllGraphHandler : public PostHandler {
public:
    bool run(std::string_view data, std::string &resultStr) override;

private:
    static void
    SetResponse(std::unordered_map<std::string, std::vector<std::string>> &graphInfoMap, std::string &result);
};
}

#endif //BOARD_PLUGINS_SCALARVISUALLY_SRC_PLUGIN_HANDLER_SCALARVISUALLYGETALLGRAPHHANDLER_H_
