/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#include "gtest/gtest.h"
#include "ParserFactory.h"
#include "proto/event.pb.h"
#include "defs/ConceptDefs.h"

using namespace Insight::Scalar::Parser;
using namespace Insight::Scalar;

class ParserTestSuit : public ::testing::Test {
protected:
    std::string pathPrefix = "../../../../";
    std::string tfeventTestFile =
            pathPrefix +
            "mindstudio-board/server/plugins/ScalarVisually/test/test_data/tfevent_data/events.out.tfevents.1728628561";

    void SetUp() override
    {
    }
};

TEST_F(ParserTestSuit, ParserFactoryGetParserInstance)
{
    std::shared_ptr<FileParser> parser = nullptr;
    parser = ParserFactory::Instance().CreateFileParse(ParseDataType::Unknown);
    EXPECT_EQ(parser, nullptr);
    parser = ParserFactory::Instance().CreateFileParse(ParseDataType::TF_EVENT);
    EXPECT_EQ(parser->type_ == ParseDataType::TF_EVENT, true);
    parser = ParserFactory::Instance().CreateFileParse(ParseDataType::MindSpore_Summary);
    EXPECT_EQ(parser->type_ == ParseDataType::MindSpore_Summary, true);
    parser = ParserFactory::Instance().CreateFileParse(ParseDataType::TEXT_LOG);
    EXPECT_EQ(parser->type_ == ParseDataType::TEXT_LOG, true);
    parser = ParserFactory::Instance().CreateFileParse(ParseDataType::Unknown);
    EXPECT_EQ(parser, nullptr);
}

TEST_F(ParserTestSuit, ParseTFevent)
{
    auto parser = ParserFactory::Instance().CreateFileParse(ParseDataType::TF_EVENT);
    uint64_t offset = 0;
    auto res = parser->ParserData(tfeventTestFile, offset);
    EXPECT_EQ(res.empty(), false);
    EXPECT_EQ(offset, 18446744073709551615ull);
    EXPECT_EQ(res.size(), 1);
    EXPECT_EQ(res.count("Loss/train"), 1);
    std::vector<ScalarPoint> &datas = res.at("Loss/train");
    // The expected value is the loss value from the training process, which is 0.136831999 in this test case.
    EXPECT_FLOAT_EQ(datas[0].value_, 0.136831999);
}

TEST_F(ParserTestSuit, ParseSummaryData)
{
    auto parser = ParserFactory::Instance().CreateFileParse(ParseDataType::MindSpore_Summary);
}
