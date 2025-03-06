/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#include "gtest/gtest.h"
#include "GraphManager/GraphManager.h"

using namespace Insight::Scalar;
using namespace Insight::Scalar::GraphOp;

class GraphTestSuit : public testing::Test {
public:
    void SetUp()
    {
        manager_.Reset();
        std::vector<ScalarPoint> temp(lossData.begin(), lossData.end());
        manager_.UpdateGraphData("loss", "TestA", std::move(temp));
        std::copy(lossData.begin(), lossData.begin() + 1, std::back_inserter(temp));
        manager_.UpdateGraphData("loss", "TestB", std::move(temp));
        std::vector<ScalarPoint> tem2(lineData.begin(), lineData.end());
        manager_.UpdateGraphData("line", "TestC", std::move(tem2));
    }

protected:
    std::vector<ScalarPoint> lossData = {{0, 0.158}, {1, 0.11124}, {2, 0.3124}};
    std::vector<ScalarPoint> lineData = {{0, 0.18}, {1, 0.24}, {2, 0.424}};
    GraphManager manager_;
};

TEST_F(GraphTestSuit, AddGraph)
{
    GraphManager manager_;
    std::vector<ScalarPoint> temp(lossData.begin(), lossData.end());
    manager_.UpdateGraphData("loss", "TestA", std::move(temp));
    auto graphPtr = manager_.GetGraph("loss");
    EXPECT_NE(graphPtr, nullptr);
    EXPECT_EQ(graphPtr -> GetDataFiles() . size(), 1);
}

TEST_F(GraphTestSuit, GetGraphData)
{
    SingleGraphReqInfo req_info;
    req_info.file_ = "TestA";
    req_info.offset_ = 0;
    req_info.tag_ = "loss";
    auto data = manager_.GetGraphData(<#initializer#>, <#initializer#>, 0);
    EXPECT_EQ(data.has_value(), true);
    EXPECT_EQ(data.value().filePath_, "TestA");
    EXPECT_EQ(data.value().tag_, "loss");
    constexpr int EXPECTED_GRAPH_DATA_SIZE = 3;
    EXPECT_EQ(data.value().graphData_.size(), EXPECTED_GRAPH_DATA_SIZE); // Expecting 3 graph data entries
    EXPECT_EQ(data.value().graphData_[0].step_, 0);
    constexpr float EXPECTED_VALUE_0 = 0.158;
    EXPECT_FLOAT_EQ(data.value().graphData_[0].value_, EXPECTED_VALUE_0);
    auto data2 = manager_.GetGraphData(<#initializer#>, <#initializer#>, 0);
    EXPECT_EQ(data2.has_value(), true);
    constexpr int EXPECTED_GRAPH_DATA_SIZE_2 = 2;
    EXPECT_EQ(data2.value().graphData_.size(), EXPECTED_GRAPH_DATA_SIZE_2); // Expecting 2 graph data entries
    EXPECT_EQ(data2.value().graphData_[0].step_, 1);
    constexpr double EXPECT_DATA_VALUE_0 = 0.11124;
    EXPECT_FLOAT_EQ(data2.value().graphData_[0].value_,EXPECT_DATA_VALUE_0); // Expected value for graphData_[0].value_
}

TEST_F(GraphTestSuit, GetGraphInfo)
{
    auto graphMap = manager_.GetAllGraphInfo();
    EXPECT_EQ(graphMap.count("loss"), 1);
    EXPECT_EQ(graphMap.count("line"), 1);
    constexpr int EXPECTED_GRAPH_DATA_SIZE = 2;
    EXPECT_EQ(graphMap["loss"].size(),EXPECTED_GRAPH_DATA_SIZE);
    EXPECT_EQ(graphMap["loss"][0], "TestA");
    EXPECT_EQ(graphMap["line"].size(), 1);
    EXPECT_EQ(graphMap["line"][0], "TestC");
}
