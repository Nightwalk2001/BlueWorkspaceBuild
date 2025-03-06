/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#include "gtest/gtest.h"
#include "FileManager/FileInfoManager.h"

using namespace Insight::Scalar;
using namespace Insight::Scalar::FileInfo;

class FileManagerTestSuit : public testing::Test {
protected:
};

TEST_F(FileManagerTestSuit, TestAddFile)
{
    FileInfoManager manager;
    auto fsp1 = manager.AddFile("/root/test.a", ParseDataType::TF_EVENT);
    EXPECT_EQ(manager .GetFileInfo("/root/test.a") == fsp1, true);
    EXPECT_EQ(fsp1 ->filePath_ == "/root/test.a", true);
    EXPECT_EQ(fsp1 ->offSet_, 0);
    EXPECT_EQ(fsp1 ->parseDataType_, ParseDataType::TF_EVENT);
}

TEST_F(FileManagerTestSuit, TestRepeatAddFile)
{
    FileInfoManager manager;
    auto fsp1 = manager.AddFile("/root/test.a", ParseDataType::TF_EVENT);
    auto fsp2 = manager.AddFile("/root/test.a", ParseDataType::TF_EVENT);
    EXPECT_EQ(fsp1, fsp2);
    EXPECT_EQ(fsp1 == nullptr, false);
}

TEST_F(FileManagerTestSuit, DelFile)
{
    FileInfoManager manager;
    manager.AddFile("/root/test.a", ParseDataType::TF_EVENT);
    manager.DelFile("/root/test.a");
    EXPECT_EQ(manager .GetFileInfo("/root/test.a") == nullptr, true);
}
