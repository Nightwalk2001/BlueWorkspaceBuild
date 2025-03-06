/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef BOARD_HTTPSERVER_H
#define BOARD_HTTPSERVER_H

#include <memory>
#include <string_view>
#include "App.h"
#include "ApiHandler.h"

namespace Insight::Http {
using namespace uWS;
using namespace Dic::Core;

class HttpServer {
public:
    static HttpServer &Instance();

    bool Start();

private:
    HttpServer() = default;

    ~HttpServer() = default;

    void AddApiHandlers();

    void AddGetHandler(std::string_view key, const std::shared_ptr<ApiHandler> handler);

    void AddPostHandler(std::string_view key, const std::shared_ptr<ApiHandler> handler);

    static std::string GetBasicResult();

    std::unique_ptr<App> wsApp;
    uint16_t port{6065};
};
}

#endif // BOARD_HTTPSERVER_H
