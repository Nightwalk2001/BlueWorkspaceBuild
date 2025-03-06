/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#include <sstream>
#include <iostream>
#include "stringbuffer.h"
#include "writer.h"
#include "HttpServer.h"
#include "PluginsManager.h"
#include "Logger.h"

using namespace Insight;
namespace Insight::Http {
using json_t = rapidjson::Value;
using document_t = rapidjson::Document;

HttpServer &HttpServer::Instance()
{
    static HttpServer instance;
    return instance;
}

static inline std::string DumpJsonToStr(json_t &jsonSrc)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    jsonSrc.Accept(writer);
    return {buffer.GetString(), buffer.GetSize()};
}

bool HttpServer::Start()
{
    wsApp = std::make_unique<App>();
    AddApiHandlers();
    wsApp->options("/*", [](auto *res, auto *req) {
        res->end();
    });
    wsApp->listen("0.0.0.0", port, [](auto *token) {
        if (token) {
            LOG(LogRank::Info) << "http server start";
        }
    }).run();
    return false;
}

void HttpServer::AddApiHandlers()
{
    wsApp->get("test", [this](HttpResponse<false> *res, auto *req) {
        std::cout << "test" << std::endl;
        res->tryEnd("", 0);
    });
    auto &manager = Dic::Core::PluginsManager::Instance();
    Dic::Core::PluginsManager::LoadPlugins();
    for (const auto &[name, plugin]: manager.GetAllPlugins()) {
        auto handlers = plugin->GetAllHandlers();
        for (const auto &[key, handler]: handlers) {
            std::cout << "Add Handler:" << key << std::endl;
            if (handler->GetApiType() == Dic::Core::API_TYPE::GET) {
                std::cout << "Add Handler2" << std::endl;
                AddGetHandler(std::string("/" + name + "/" + key), handler);
            } else {
                AddPostHandler(std::string("/" + name + "/" + key), handler);
            }
        }
    }
}

void HttpServer::AddGetHandler(std::string_view key, const std::shared_ptr<Dic::Core::ApiHandler> handler)
{
    wsApp->get(key.data(), [handler, this](HttpResponse<false> *res, HttpRequest *req) {
        res->writeHeader("Access-Control-Allow-Origin", "*"); // allow CROS request
        res->writeHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res->writeHeader("Access-Control-Allow-Headers", "Content-Type");
        res->writeHeader("Access-Control-Allow-Credentials", "true");
        std::string result = GetBasicResult();
        if (handler->run(req->getQuery(), result)) {
        }
        res->tryEnd(result, result.size());
    });
}

void HttpServer::AddPostHandler(std::string_view key, const std::shared_ptr<Dic::Core::ApiHandler> handler)
{
    wsApp->post(key.data(), [handler, this](HttpResponse<false> *res, auto *req) {
        res->onAborted([]() {
            Loop::get()->defer([]() {
            });
        });
        res->onData([res, handler, bodyBuffer = std::string(), this](std::string_view data, bool isEnd) mutable {
            bodyBuffer.append(data);
            if (isEnd) {
                // 处理数据
                res->writeHeader("Access-Control-Allow-Origin", "*"); // allow CROS request
                res->writeHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                res->writeHeader("Access-Control-Allow-Headers", "Content-Type");
                res->writeHeader("Access-Control-Allow-Credentials", "true");
                std::string result = GetBasicResult();
                bool sucess = handler->run(bodyBuffer, result);
                // long data length can't send rightly with try end
                res->end(result, true);
                LOG(LogRank::Info) << "Response size:" << result.size();
            }
        });
    });
}

std::string HttpServer::GetBasicResult()
{
    return R"({"body":{}, "msg":"", "errCode":0, "result":true})";
}
}
