/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#include "TFEventParser.h"
#include "proto/event.pb.h"
#include "Logger.h"

using namespace Insight::Scalar::Parser;
using namespace Insight::Scalar;
using namespace Insight;

bool TFEventParser::EventContainsScalar(const tensorboard::Event &event) {
    if (!event.has_summary()) {
        return false;
    }
    const auto &summary = event.summary();
    bool hasScalarValue =
            std::any_of(summary.value().begin(), summary.value().end(), [](const tensorboard::Summary::Value &item) {
                return item.value_case() == tensorboard::Summary::Value::kSimpleValue;
            });
    if (!hasScalarValue) {
        return false;
    }
    return true;
}

bool TFEventParser::ReadRecord(std::string &eventStr, std::ifstream &input) {
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
    length = *(uint64_t *)record.data();
#endif
    if (!ReadCheckSumRecord(input, record, length)) {
        return false;
    }
    eventStr = std::string(record.data(), length);
    return true;
}

bool TFEventParser::ParseRecordToScalar(std::string &&record, std::map<std::string, std::vector<ScalarPoint>> &res) {
    tensorboard::Event event;
    if (!event.ParseFromString(record)) {
        LOG(LogRank::Error) << "Can't convert str to tensorflow event";
        return false;
    }
    if (!EventContainsScalar(event)) {
        // event not contains scalar data, skip
        return true;
    }
    const int64_t step = event.step();
    const double wallTime = event.wall_time();
    const tensorboard::Summary &summary = event.summary();
    for (const auto &value: summary.value()) {
        if (value.value_case() != tensorboard::Summary::Value::kSimpleValue) {
            continue;
        }
        const std::string &tag = value.tag();
        const float scalarValue = value.simple_value();
        res[tag].emplace_back(step, scalarValue, wallTime);
    }
    return true;
}

