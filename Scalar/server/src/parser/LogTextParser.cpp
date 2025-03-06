/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#include "LogTextParser.h"
#include <regex>
#include "Logger.h"
#include "ScalarVisuallyServer.h"

using namespace Insight::Scalar;
using namespace Insight::Scalar::Parser;

bool LogTextParser::ReadRecord(std::string &recordStr, std::ifstream &input) {
    getline(input, recordStr);
    return (!input.eof() && !input.bad());
}

bool LogTextParser::ParseRecordToScalar(std::string &&record, std::map<std::string, std::vector<ScalarPoint>> &res) {
    if (record.empty()) {
        return true;
    }
    // parse step information
    if (record.find("step") == std::string::npos) {
        return true;
    }
    constexpr size_t MATCH_INDEX = 1;
    std::string date;
    const static auto timeRegex = std::regex(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2},\d{3})");
    if (std::smatch smatch; std::regex_search(record, smatch, timeRegex)) {
        if (!smatch.empty() && smatch[0].matched) {
            date = smatch[0].str();
        }
    }
    int64_t step = 0;
    const static auto stepRegex = std::regex(R"(step:\s*\[\s*(\d+)/\s*(\d+)\])");
    if (std::smatch smatch; std::regex_search(record, smatch, stepRegex)) {
        if (smatch.size() > MATCH_INDEX && smatch[MATCH_INDEX].matched) {
            step = std::stoll(smatch[MATCH_INDEX].str());
        }
    } else {
        return true;
    }
    float value = 0;
    const static auto lossRegex = std::regex(R"(loss:\s*([-+]?\d*\.\d+|\d+))");
    if (std::smatch smatch; std::regex_search(record, smatch, lossRegex)) {
        if (smatch.size() > 1 && smatch[1].matched) {
            value = std::stof(smatch[1].str());
            res[TEXT_DEFAULT_TAG_LOSS.data()].emplace_back(step, value, date);
        }
    }
    const static auto normalRegex = std::regex(R"(global_norm:\s*\[([-+]?\d*\.\d+|\d+))");
    if (std::smatch smatch; std::regex_search(record, smatch, normalRegex)) {
        if (smatch.size() > 1 && smatch[1].matched) {
            value = std::stof(smatch[1].str());
            res[TEXT_DEFAULT_TAG_GLOBAL_NORM.data()].emplace_back(step, value, date);
        }
    }
    return true;
}

void LogTextParser::BeforeParse()
{
    auto fileInfo = ScalarVisuallyServer::Instance().GetFileInfo(fileName_);
    if (fileInfo == nullptr) {
        return;
    }
    auto file = OpenFileSafe(fileName_);
    if (!file.is_open()) {
        return;
    }
    // read until find seqlength
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("global_batch_size") == std::string::npos) {
            continue;
        }
        static std::regex globalBatchSizeRegex(R"(\d+$)");
        if (std::smatch match;std::regex_search(line, match, globalBatchSizeRegex)) {
            if (match.size() > 0 && match[0].matched) {
                fileInfo->SetGlobalBatchSize(std::stod(match[0].str()));
                break;
            }
        }
    }
}
