/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#include "SummaryParser.h"
#include "Logger.h"
#include "mindspore_summary.pb.h"

using namespace Insight::Scalar::Parser;
using namespace Insight::Scalar;
using namespace Insight;

bool SummaryParser::EventContainsScalar(const mindspore::irpb::Event &event) {
    if (!event.has_summary()) {
        return false;
    }
    const mindspore::irpb::Summary &summary = event.summary();
    bool hasScalarValue =
            std::any_of(summary.value().begin(), summary.value().end(),
                        [](const mindspore::irpb::Summary::Value &value) {
                            return value.value_case() == mindspore::irpb::Summary::Value::kScalarValue;
                        });
    if (!hasScalarValue) {
        return false;
    }
    return true;
}

bool SummaryParser::ReadRecord(std::string &eventStr, std::ifstream &input) {
    /*
 *  The structure of tf event file:
 *  | data length | CRC sum  | pb data | CRC sum  |
 *  | uint64_t    | uint32_t | ....    | uint32_t |
 */
    std::vector<char> record;
    if (!ReadCheckSumRecord(input, record, sizeof(uint64_t))) {
        return false;
    }
    uint64_t length = 0;
#ifdef _WIN32
    memcpy_s(&length, sizeof(length), record.data(), sizeof(uint64_t));
#else
    length = *((uint64_t *)record.data());
#endif
    if (!ReadCheckSumRecord(input, record, length)) {
        return false;
    }
    eventStr = std::string(record.data(), length);
    return true;
}

bool SummaryParser::ParseRecordToScalar(std::string &&record, std::map<std::string, std::vector<ScalarPoint>> &res) {
    mindspore::irpb::Event event;
    if (!event.ParseFromString(record)) {
        LOG(LogRank::Error) << "Can't convert str to mindspore event";
        return false;
    }
    if (!EventContainsScalar(event)) {
        // not contains scalar, skip
        return true;
    }
    const int64_t step = event.step();
    const double wallTime = event.wall_time();
    for (const auto &value: event.summary().value()) {
        if (value.value_case() != mindspore::irpb::Summary::Value::kScalarValue) {
            continue;
        }
        const std::string &tag = value.tag();
        float scalarValue = value.scalar_value();
        res[tag].emplace_back(step, scalarValue, wallTime);
    }
    return true;
}
