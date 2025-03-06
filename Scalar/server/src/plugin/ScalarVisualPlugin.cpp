
/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#include "ScalarVisualPlugin.h"
#include "PluginsManager.h"
#include "Handler/ScalarVisuallyImportFileHandler.h"
#include "Handler/ScalarVisuallyGetAllGraphHandler.h"
#include "Handler/ScalarVisuallyGetScalarDataHandler.h"
#include "Handler/ScalarVisuallyQueryCreatedFile.h"
#include "Handler/ScalarVisuallyGetParseState.h"
#include "Handler/ScalarVisuallyGetIncrementalTag.h"
#include "Handler/ScalarVisuallyFileMergeHandler.h"
#include "Handler/ScalarVisuallyTokenSet.h"

using namespace Insight;
namespace Insight::Scalar {
ScalarVisualPlugin::ScalarVisualPlugin() : Dic::Core::BasePlugin("ScalarVisually") {
    handlers_.emplace("ImportFile", std::make_shared<ScalarVisuallyImportFileHandler>());
    handlers_.emplace("GetAllGraph", std::make_shared<ScalarVisuallyGetAllGraphHandler>());
    handlers_.emplace("GetScalarData", std::make_shared<ScalarVisuallyGetScalarDataHandler>());
    handlers_.emplace("GetAddFiles", std::make_shared<ScalarVisuallyQueryCreatedFile>());
    handlers_.emplace("GetParseState", std::make_shared<ScalarVisuallyGetParseState>());
    handlers_.emplace("GetIncrementalTag", std::make_shared<ScalarVisuallyGetIncrementalTag>());
    handlers_.emplace("FileMerge", std::make_shared<ScalarVisuallyFileMergeHandler>());
    handlers_.emplace("TokenParamSet", std::make_shared<ScalarVisuallyTokenSet>());
}

std::map<std::string, std::shared_ptr<ApiHandler>> ScalarVisualPlugin::GetAllHandlers() {
    std::map<std::string, std::shared_ptr<ApiHandler>> res(handlers_.begin(), handlers_.end());
    return res;
}

std::vector<std::string> ScalarVisualPlugin::GetModuleConfig() {
    std::vector<std::string> res;
    document_t moduleConfig;
    moduleConfig.SetObject();
    auto &allocator = moduleConfig.GetAllocator();
    AddJsonMember(moduleConfig, "name", "Scalar", allocator);
    AddJsonMember(moduleConfig, "requestName", "scalar", allocator);
    json_t attributes(rapidjson::kObjectType);
    AddJsonMember(attributes, "src", "./plugins/Scalar/index.html", allocator);
    AddJsonMember(moduleConfig, "attributes", attributes, allocator);
    AddJsonMember(moduleConfig, "isDefault", true, allocator);
    AddJsonMember(moduleConfig, "isCluster", true, allocator);
    AddJsonMember(moduleConfig, "isCompute", true, allocator);
    AddJsonMember(moduleConfig, "isJupyter", true, allocator);
    res.push_back(DumpJsonToStr(moduleConfig));
    return res;
}

}

PluginRegister pluginRegister(std::move(std::make_unique<Insight::Scalar::ScalarVisualPlugin>()));
