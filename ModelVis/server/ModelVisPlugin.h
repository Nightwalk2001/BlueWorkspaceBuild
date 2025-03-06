#pragma once

#include "BasePlugin.h"
#include "document.h"

using json = rapidjson::Value;
using document_t = rapidjson::Document;

using namespace Dic::Core;

class ModelVisPlugin final : public BasePlugin {
public:
    ModelVisPlugin();

    ~ModelVisPlugin() override = default;

    std::map<std::string, std::shared_ptr<ApiHandler> > GetAllHandlers() override;

    std::vector<std::string> GetModuleConfig() override;

private:
    std::map<std::string, std::shared_ptr<ApiHandler> > handlers;
};
