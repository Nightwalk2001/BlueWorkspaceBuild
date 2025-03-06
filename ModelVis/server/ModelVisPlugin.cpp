#include "ModelVisPlugin.h"
#include "PluginsManager.h"
#include "ModelVisHandlers.hpp"

template<typename T>
static void AddJsonMember(
    json &dst,
    const std::string_view key,
    T &&value,
    rapidjson::MemoryPoolAllocator<> &allocator
) {
    dst.AddMember(
        json().SetString(key.data(), allocator),
        std::forward<T>(value),
        allocator);
}

static std::string DumpJsonToStr(const json &document) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer writer(buffer);
    document.Accept(writer);
    return {buffer.GetString(), buffer.GetSize()};
}

ModelVisPlugin::ModelVisPlugin(): BasePlugin("ModelVisual") {
    handlers.emplace("Layout", std::make_shared<LayoutHandler>());
    handlers.emplace("MiningSubGraph", std::make_shared<MiningSubGraphHandler>());
}

std::map<std::string, std::shared_ptr<ApiHandler> > ModelVisPlugin::GetAllHandlers() {
    std::map res(handlers.begin(), handlers.end());
    return res;
}

std::vector<std::string> ModelVisPlugin::GetModuleConfig() {
    std::vector<std::string> res;

    document_t moduleConfig;
    moduleConfig.SetObject();
    auto &allocator = moduleConfig.GetAllocator();

    AddJsonMember(moduleConfig, "name", "ModelVis", allocator);
    AddJsonMember(moduleConfig, "requestName", "modelvis", allocator);
    json attributes(rapidjson::kObjectType);
    AddJsonMember(attributes, "src", "./plugins/ModelVis/index.html", allocator);
    AddJsonMember(moduleConfig, "attributes", true, allocator);
    AddJsonMember(moduleConfig, "isCluster", attributes, allocator);
    AddJsonMember(moduleConfig, "isDefault", true, allocator);
    AddJsonMember(moduleConfig, "isCompute", true, allocator);
    AddJsonMember(moduleConfig, "isJupyter", true, allocator);
    res.push_back(DumpJsonToStr(moduleConfig));

    return res;
}

PluginRegister pluginRegister(std::make_unique<ModelVisPlugin>());
