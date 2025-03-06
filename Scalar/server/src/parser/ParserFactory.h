/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef PARSERFACTORY_H
#define PARSERFACTORY_H

#include <memory>
#include <functional>
#include "LogTextParser.h"
#include "SummaryParser.h"
#include "TFEventParser.h"

namespace Insight::Scalar::Parser {
class ParserFactory {
public:
    ParserFactory(const ParserFactory &) = delete;
    ParserFactory &operator=(const ParserFactory &) = delete;
    ParserFactory(ParserFactory &&) = delete;

    static ParserFactory &Instance() {
        static ParserFactory instance;
        return instance;
    }

    std::shared_ptr<FileParser> CreateFileParse(ParseDataType dataType) {
        if (parsers_.find(dataType) == parsers_.end()) {
            return nullptr;
        }
        return parsers_[dataType]();
    }

private:
    ParserFactory() = default;

    static inline std::map<ParseDataType, std::function<std::shared_ptr<FileParser>()> > parsers_{{ParseDataType::TF_EVENT,          []() { return std::make_shared<TFEventParser>(); }},
                                                                                                  {ParseDataType::MINDSPORE_SUMMARY, []() { return std::make_shared<SummaryParser>(); }},
                                                                                                  {ParseDataType::TEXT_LOG,          []() { return std::make_shared<LogTextParser>(); }},
                                                                                                  {ParseDataType::Unknown,           []() { return nullptr; }}
    };
};
}

#endif //PARSERFACTORY_H
