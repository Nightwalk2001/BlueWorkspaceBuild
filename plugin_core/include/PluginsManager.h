/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef BOARD_PLUGINSMANAGER_H
#define BOARD_PLUGINSMANAGER_H

#include "map"
#include "memory"

#include "BasePlugin.h"

namespace Dic::Core {
class PluginsManager {
public:
    static PluginsManager &Instance();
    bool RegisterPlugin(std::unique_ptr<BasePlugin> plugin);
    std::map<std::string, std::unique_ptr<BasePlugin>>& GetAllPlugins();
    static void LoadPlugins();
private:
    PluginsManager() = default;
    ~PluginsManager() = default;

private:
    std::map<std::string, std::unique_ptr<BasePlugin>> pluginsMap_;
};
}

#endif // BOARD_PLUGINSMANAGER_H
