/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#include <iostream>
#ifdef _WIN32
#include <filesystem>
namespace fs = std::filesystem;
#else

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
#endif
#include "gtest/gtest.h"
#include "plugin/ScalarVisualPlugin.h"
#include "ApiHandler.h"
#include "Util/ScalaryProtocolUtil.h"
#include "ScalarVisuallyServer.h"
using namespace Insight::Scalar;
using namespace Dic::Core;
using namespace Insight::Scalar::Protocol;

class PluginTestSuit : public testing::Test {
protected:
    ScalarVisualPlugin plugin_;

    void SetUpTestSuit()
    {
    };

    const std::shared_ptr<ApiHandler> &GetHandler(std::string_view name)
    {
        auto handlerMap = plugin_.GetAllHandlers();
        return handlerMap.at(std::string(name));
    }

    std::string GetBasicResult()
    {
        return R"({"body":{}, "msg":"", "errCode":0, "result":true})";
    }

    void ImportFile(std::string &file)
    {
        auto &imporFileHandler = GetHandler("ImportFile");
        std::string request = R"({"append": false, "pathList": [")" + file + R"("]})";
        std::string resultStr = GetBasicResult();
        bool success = imporFileHandler->run(request, resultStr);
    }

    std::string currentPath = fs::current_path().string();
    size_t index = currentPath.find("mindstudio-board");
    std::string pathPreFix = currentPath.substr(0, index);
    std::string tfDataDir = pathPreFix + "mindstudio-board/server/plugins/ScalarVisually/test/test_data/tfevent_data";
    std::string tfDataFile =
            pathPreFix +
            "mindstudio-board/server/plugins/ScalarVisually/test/test_data/tfevent_data/events.out.tfevents.1728628561";
    ScalarVisuallyServer &server = ScalarVisuallyServer::Instance();
};

TEST_F(PluginTestSuit, ImportFileWithDir)
{
    auto &imporFileHandler = GetHandler("/ScalarVisually/ImportFile");
    std::string request = R"({"append": false, "pathList": [")" + tfDataDir + R"(]})";
    std::string resultStr = GetBasicResult();
    bool success = imporFileHandler->run(request, resultStr);
    document_t result = ParseJsonToStr(resultStr);
    json &body = result["body"];
    EXPECT_EQ(success, true);
    EXPECT_EQ(body.HasMember("data"), true);
    json &data = body["data"];
    EXPECT_EQ(data.IsArray(), true);
    EXPECT_EQ(data.Size(), 1);
    EXPECT_EQ(data[0].HasMember("tag"), true);
    EXPECT_STREQ(data[0]["tag"].GetString(), "Loss/train");
}

TEST_F(PluginTestSuit, ImportFileWithFile)
{
    auto &imporFileHandler = GetHandler("/ScalarVisually/ImportFile");
    std::string request = R"({"append": false, "pathList": [")" + tfDataFile + R"("]})";
    std::string resultStr = GetBasicResult();
    bool success = imporFileHandler->run(request, resultStr);
    document_t result = ParseJsonToStr(resultStr);
    json &body = result["body"];
    EXPECT_EQ(success, true);
    EXPECT_EQ(body.HasMember("data"), true);
    json &data = body["data"];
    EXPECT_EQ(data.IsArray(), true);
    EXPECT_EQ(data.Size(), 1);
    EXPECT_EQ(data[0].HasMember("tag"), true);
    EXPECT_STREQ(data[0]["tag"].GetString(), "Loss/train");
}

TEST_F(PluginTestSuit, GetLossDataOffSetZero)
{
    ImportFile(tfDataFile);
    auto &GetDataHandler = GetHandler("/ScalarVisually/GetScalarData");
    std::string request = R"({"graphList": [{"tag": "Loss/train", "file": ")" + tfDataFile + R"(", "offset": 0}]})";
    std::string response = GetBasicResult();
    bool succes = GetDataHandler->run(request, response);
    EXPECT_EQ(succes, true);
    document_t result = ParseJsonToStr(response);
    json &body = result["body"];
    json &data = body["data"];
    EXPECT_EQ(data.IsArray(), true);
    json &lossData = data[0];
    EXPECT_STREQ(lossData["tag"].GetString(), "Loss/train");
    json &points = lossData["points"];
    EXPECT_EQ(points.IsArray(), true);
    constexpr int EXPECTED_POINTS_SIZE = 10000;
    EXPECT_EQ(points.Size(), EXPECTED_POINTS_SIZE); // 预期的points数组大小为10000
}

TEST_F(PluginTestSuit, GetLossDataOffSet)
{
    ImportFile(tfDataFile);
    auto &GetDataHandler = GetHandler("/ScalarVisually/GetScalarData");
    std::string request = R"({"graphList": [{"tag": "Loss/train", "file": ")" + tfDataFile + R"(", "offset": 10}]})";
    std::string response = GetBasicResult();
    bool succes = GetDataHandler->run(request, response);
    EXPECT_EQ(succes, true);
    document_t result = ParseJsonToStr(response);
    json &body = result["body"];
    json &data = body["data"];
    EXPECT_EQ(data.IsArray(), true);
    json &lossData = data[0];
    EXPECT_STREQ(lossData["tag"].GetString(), "Loss/train");
    json &points = lossData["points"];
    EXPECT_EQ(points.IsArray(), true);
    constexpr int EXPECTED_POINTS_SIZE = 9990;
    EXPECT_EQ(points.Size(), EXPECTED_POINTS_SIZE);
}

TEST_F(PluginTestSuit, GetLossDataOffSetExceedSize)
{
    ImportFile(tfDataFile);
    auto &GetDataHandler = GetHandler("/ScalarVisually/GetScalarData");
    std::string request = R"({"graphList": [{"tag": "Loss/train", "file": ")" + tfDataFile + R"(", "offset": 10010}]})";
    std::string response = GetBasicResult();
    bool succes = GetDataHandler->run(request, response);
    EXPECT_EQ(succes, true);
    document_t result = ParseJsonToStr(response);
    json &body = result["body"];
    json &data = body["data"];
    EXPECT_EQ(data.IsArray(), true);
    json &lossData = data[0];
    EXPECT_STREQ(lossData["tag"].GetString(), "Loss/train");
    json &points = lossData["points"];
    EXPECT_EQ(points.IsArray(), true);
    EXPECT_EQ(points.Size(), 0);
}

TEST_F(PluginTestSuit, GetAllGraph)
{
    ImportFile(tfDataFile);
    auto &handler = GetHandler("/ScalarVisually/GetAllGraph");
    std::string request = R"()";
    std::string response = GetBasicResult();
    bool success = handler->run(request, response);
    EXPECT_EQ(success, true);
    document_t result = ParseJsonToStr(response);
    json &body = result["body"];
    json &data = body["data"];
    EXPECT_EQ(data.IsArray(), true);
    EXPECT_EQ(data.Size(), 1);
    json &graph = data[0];
    EXPECT_STREQ(graph["tag"].GetString(), "Loss/train");
    EXPECT_EQ(graph["fileList"].IsArray() && graph["fileList"].Size() == 1, true);
    json &file = graph["fileList"][0];
    EXPECT_EQ(file.IsString(), true);
    EXPECT_EQ(tfDataFile.compare(file.GetString()) == 0, true);
}

TEST_F(PluginTestSuit, ImportNewFile)
{
    server.OnFileCreate(std::string(tfDataDir), fs::path(tfDataFile).filename().string());
    std::string request = R"({"pathList":[")" + tfDataFile + R"("], "append":true})";
    std::string result = GetBasicResult();
    auto handler = GetHandler("ImportFile");
    auto errCode = handler->run(request, result);
    EXPECT_EQ(errCode, 0);
}
