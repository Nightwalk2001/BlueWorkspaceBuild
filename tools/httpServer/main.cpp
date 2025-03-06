/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */

#include "HttpServer.h"

using namespace Dic::Core;

int main(int argc, char *argv[])
{
    Insight::Http::HttpServer::Instance().Start();
}