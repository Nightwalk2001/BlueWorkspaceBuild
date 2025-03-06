/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef MSINSIGHT_WSSENDER_H
#define MSINSIGHT_WSSENDER_H
#include "ProtocolUtil.h"
namespace Dic{
void SendEvent(std::unique_ptr<Dic::Protocol::Event> eventPtr);
void SendResponse(std::unique_ptr<Protocol::Response> responsePtr, bool result, const std::string &errorMsg = "", const int errorCode = UNKNOW_ERROR);
}
#endif //MSINSIGHT_WSSENDER_H
