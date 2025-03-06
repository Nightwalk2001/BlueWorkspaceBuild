/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#include "FileParser.h"
#include "Logger.h"
#include "defs/ConceptDefs.h"
#include "ScalarVisuallyServer.h"

using namespace Insight::Scalar::Parser;
using namespace Insight::Scalar;
using namespace Insight;

bool FileParser::CheckFilePathVaild(const std::string &filePath) {
    if (filePath.empty()) {
        LOG(LogRank::Error) << "File path is empty";
        return false;
    }
    if (!fs::exists(filePath)) {
        LOG(LogRank::Error) << "File path  does not exist";
        return false;
    }
    auto readPermission = fs::status(filePath).permissions() & fs::perms::owner_read;
    if (readPermission == fs::perms::none) {
        LOG(LogRank::Error) << "File not permit to read";
        return false;
    }
    return true;
}

std::ifstream FileParser::OpenFileSafe(const std::string &filePath) {
    std::ifstream file;
    fileName_ = filePath;
    file.setstate(std::ios::badbit);
    if (!CheckFilePathVaild(filePath)) {
        LOG(LogRank::Error) << "Open file failed";
        return file;
    }
    file = std::ifstream(filePath, std::ios::in | std::ios::binary);
    return file;
}

bool FileParser::ParserData(const std::string &filePath, uint64_t &offset)
{
    std::ifstream file = OpenFileSafe(filePath);
    if (!file.is_open()) {
        LOG(LogRank::Warning) << "Parse data faild, open file error";
        return true;
    }
    BeforeParse();
    file.seekg(static_cast<int64_t>(offset), std::ios::beg);
    std::string recordStr;
    bool flag = true;
    while (file && ReadRecord(recordStr, file)) {
        std::map<std::string, std::vector<ScalarPoint> > data;
        if (!ParseRecordToScalar(std::move(recordStr), data)) {
            break;
        }
        std::streampos index = file.tellg();
        if (index != -1) {
            offset = index;
        }
        if (data.empty()) {
            continue;
        }
        flag = false;
        for (auto &[tag, points]: data) {
            if (!UpdateData(filePath, tag, std::move(points))) {
                return false;
            }
        }
    }
    UpdateData(filePath, "", {}, true);  // parsed end ,update all data to graph
    return flag;
}

bool FileParser::ReadCheckSumRecord(std::ifstream &input, std::vector<char> &buffer, size_t size) {
    if (!input) {
        return false;
    }
    if (size > std::numeric_limits<int64_t>::max()) {
        LOG(LogRank::Error) << "Read data exceed limit";
        return false;
    }

    buffer.clear();
    buffer.resize(size + 1);
    input.read(buffer.data(), static_cast<int64_t>(size));
    if (input.gcount() != size) {
        return false;
    }
    uint32_t ccrc = 0;
    input.read(reinterpret_cast<char *>(&ccrc), sizeof(uint32_t));
    if (input.gcount() != sizeof(uint32_t)) {
        return false;
    }
    return true;
}

bool FileParser::UpdateData(const std::string &filePath, const std::string &tag, std::vector<ScalarPoint> &&points,
                            bool force)
{
    if (!tag.empty()) {
        tagSet_.insert(tag);
        dataCount_ += points.size();
        dataCache_.emplace(filePath, tag, std::move(points));
    }
    if (ReachLimit() || force) {
        return UpdateDataToGraph();
    }
    return true;
}

bool FileParser::ReachLimit()
{
    constexpr uint32_t maxTagCount = 100;
    constexpr uint32_t maxDataCount = 10000;
    if (tagSet_.size() > maxTagCount || dataCount_ > maxDataCount) {
        return true;
    }
    return false;
}

bool FileParser::UpdateDataToGraph()
{
    while (!dataCache_.empty()) {
        auto &[filePath, tag, points] = dataCache_.front();
        if (!ScalarVisuallyServer::Instance().IsFileImported(filePath)) {
            return false;
        }
        if (ScalarVisuallyServer::Instance().IsIncremental(filePath, tag)) {
            ScalarVisuallyServer::Instance().AddIncremental(filePath, tag);
        }
        ScalarVisuallyServer::Instance().UpdateGraph(tag, filePath, std::move(points));
        dataCache_.pop();
    }
    tagSet_.clear();
    dataCount_ = 0;
    return true;
}
