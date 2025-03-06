#include "PluginsManager.h"
#include <string>

using namespace Dic::Core;
#ifdef _WIN32
#include <filesystem>
#include <Windows.h>
namespace fs = std::filesystem;
const static  std::string EXT = ".dll";
#else

#include <dlfcn.h>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
const static std::string EXT = ".so";
#endif

PluginsManager &PluginsManager::Instance()
{
    static PluginsManager instance;
    return instance;
}

bool PluginsManager::RegisterPlugin(std::unique_ptr<BasePlugin> plugin)
{
    pluginsMap_.emplace(plugin->GetPluginName(), std::move(plugin));
    return true;
}

void PluginsManager::LoadPlugins()
{
    auto pluginsDir = PLUGINS_DIR;
    if (!fs::exists(pluginsDir)) {
        return;
    }
    for (auto &dir: fs::directory_iterator(pluginsDir)) {
        if (!fs::is_directory(dir)) {
            continue;
        }
        for (auto &file: fs::directory_iterator(dir)) {
            if (!fs::is_directory(file) && file.path().extension().string() == EXT) {
#ifdef _WIN32
                LoadLibraryA(file.path().string().c_str());
#else
                dlopen(file.path().string().c_str(), RTLD_LAZY);
#endif
            }
        }
    }

}

std::map<std::string, std::unique_ptr<BasePlugin>> &PluginsManager::GetAllPlugins()
{
    return pluginsMap_;
}

PluginRegister::PluginRegister(std::unique_ptr<BasePlugin> plugin)
{
    PluginsManager::Instance().RegisterPlugin(std::move(plugin));
}
