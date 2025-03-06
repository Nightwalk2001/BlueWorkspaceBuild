/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef MSINSIGHT_BASEMODULE_H_
#define MSINSIGHT_BASEMODULE_H_
#include "ProtocolUtil.h"
#include "ModuleRequestHeadler.h"

namespace Dic::Module {
class BaseModule {
public:
    BaseModule() = default;

    virtual ~BaseModule() = default;

    virtual void RegisterRequestHandlers() = 0;

    virtual void OnRequest(std::unique_ptr<Protocol::Request> request);

protected:
    std::string moduleName = MODULE_UNKNOWN;
    std::map<std::string, std::unique_ptr<ModuleRequestHandler> > requestHandlerMap;
};
}

#endif //BOARD_MINDSTUDIO_BOARD_SERVER_CORE_INCLUDE_BASEMODULE_H_
