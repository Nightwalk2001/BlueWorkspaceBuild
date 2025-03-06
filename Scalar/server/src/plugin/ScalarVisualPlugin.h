/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#ifndef BOARD_SCALARVISUALPLUGIN_H
#define BOARD_SCALARVISUALPLUGIN_H

#include "BasePlugin.h"

namespace Insight::Scalar {
using namespace Dic::Core;

class ScalarVisualPlugin : public BasePlugin {
public:
    ScalarVisualPlugin();
    ~ScalarVisualPlugin() override = default;
    std::map<std::string, std::shared_ptr<ApiHandler>> GetAllHandlers() override;
    std::vector<std::string> GetModuleConfig() override;


private:
    std::map<std::string, std::shared_ptr<ApiHandler>> handlers_;
};
}

#endif //BOARD_SCALARVISUALPLUGIN_H
