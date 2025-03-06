/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <cstdint>
#include <fstream>
#include <optional>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include "defs/ConceptDefs.h"

namespace Insight::Scalar::Parser {
/**
 * @brief  This base class of data file parser
 */
class FileParser {
public:
    FileParser() : type_(ParseDataType::Unknown)
    {}

    /**
     * @brief open file stream safely
     * @param filePath
     * @return
     */
    std::ifstream OpenFileSafe(const std::string &filePath);

    /**
     * @brief parse data top function
     * @param filePath
     * @param[in/out] offset file read offset, update after parsed
     * @return
     */
    bool ParserData(const std::string &filePath, uint64_t &offset);

    /**
     * @brief read one record form file
     * @param recordStr
     * @param input
     * @return
     */
    virtual bool ReadRecord(std::string &recordStr, std::ifstream &input) = 0;

    /**
     * @brief parse the record to scalar data
     * @param record
     * @param res
     * @return
     */
    virtual bool ParseRecordToScalar(std::string &&record, std::map<std::string, std::vector<ScalarPoint>> &res) = 0;

    /**
     * @brief read record implement
     * @param input
     * @param buffer
     * @param size
     * @return
     */
    static bool ReadCheckSumRecord(std::ifstream &input, std::vector<char> &buffer, size_t size);

    virtual ~FileParser() = default;

    bool UpdateData(const std::string &filePath, const std::string &tag, std::vector<ScalarPoint> &&points,
                    bool force = false);

    virtual void BeforeParse()
    {};

private:
    bool CheckFilePathVaild(const std::string &filePath);

    bool ReachLimit();

    bool UpdateDataToGraph();

protected:
    std::string fileName_;
    uint32_t dataCount_{0};
    std::set<std::string> tagSet_;
    std::queue<std::tuple<std::string, std::string, std::vector<ScalarPoint>>> dataCache_;

public:
    ParseDataType type_;
};
}

#endif //FILEPARSER_H
