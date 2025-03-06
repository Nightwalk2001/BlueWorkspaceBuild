/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef SUMMARYPARSER_H
#define SUMMARYPARSER_H
#include <mindspore_summary.pb.h>
#include "FileParser.h"

namespace Insight::Scalar::Parser {

class SummaryParser : public FileParser {
public:
    SummaryParser() {
        type_ = ParseDataType::MINDSPORE_SUMMARY;
    }

    bool ReadRecord(std::string &eventStr, std::ifstream &input) override;

    bool ParseRecordToScalar(std::string &&record, std::map<std::string, std::vector<ScalarPoint>> &res) override;

    ~SummaryParser() override = default;

private:
    /**
     * @brief check event wether contains scalar data
     * @param event
     * @return
     */
    static bool EventContainsScalar(const mindspore::irpb::Event &event);
};

}

#endif //SUMMARYPARSER_H
