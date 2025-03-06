/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef LOGTEXTPARSER_H
#define LOGTEXTPARSER_H
#include <string_view>
#include "FileParser.h"

namespace Insight::Scalar::Parser {
constexpr std::string_view TEXT_DEFAULT_TAG_LOSS = "Loss";
constexpr std::string_view TEXT_DEFAULT_TAG_GLOBAL_NORM = "global_norm";
class LogTextParser : public FileParser {
public:
    LogTextParser() {
        type_ = ParseDataType::TEXT_LOG;
    }

    bool ReadRecord(std::string &recordStr, std::ifstream &input) override;

    bool ParseRecordToScalar(std::string &&record, std::map<std::string, std::vector<ScalarPoint>> &res) override;

    void BeforeParse() override;
    ~LogTextParser() override = default;
};
}

#endif //LOGTEXTPARSER_H
