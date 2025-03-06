/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef FILEINFOMANAGER_H
#define FILEINFOMANAGER_H

#include <string>
#include <set>
#include <unordered_map>
#include <utility>
#include <regex>
#include "ParserFactory.h"
#include "File.h"

namespace Insight::Scalar::FileInfo {
enum class UpdateType {
    ADD,
    DEL
};

class FileInfoManager {
public:
    /**
     * @brief add file object, return the ptr of new object, if already exist, return the ptr old object
     * @param filePath
     * @param dataType
     * @return
     */
    std::shared_ptr<File> AddFile(const std::string &filePath, ParseDataType dataType);

    /**
     * @brief delete file object, if success return true
     * @param filePath
     * @return
     */
    bool DelFileFromMap(std::string_view filePath);

    std::shared_ptr<File> GetFileInfo(std::string_view filePath);

    void Reset();

    void OnFileCreate(std::string &&dir, std::string &&fileName);

    std::unordered_map<std::string, std::set<std::string>> GetCreatedFileGroupByDir();

    static inline ParseDataType GetFileType(std::string_view filePath) {
        for (const auto &[k, v]: fileTypeMap_) {
            std::regex regex(k);
            std::smatch match;
            std::string fileName = fs::path(filePath).filename().string();
            if (std::regex_search(fileName, match, regex)) {
                return v;
            }
        }
        return ParseDataType::Unknown;
    }

    static inline bool IsFileSupported(std::string_view path) {
        return std::any_of(fileTypeMap_.begin(), fileTypeMap_.end(), [&path](auto fileTypeIter) {
            std::string mathRegex = fileTypeIter.first;
            std::regex regex(mathRegex.data());
            std::smatch match;
            std::string fileName = fs::path(path).filename().string();
            if (!std::regex_search(fileName, match, regex)) {
                return false;
            }
            return true;
        });
    }

    void AddFile(std::shared_ptr<File> fileInfo)
    {
        if (fileInfo == nullptr) {
            return;
        }
        if (filePaths_.count(fileInfo->FilePath()) != 0) {
            return;
        }
        fileInfoMap_.emplace(fileInfo->FilePath(), fileInfo);
        filePaths_.insert(fileInfo->FilePath());
    }
private:
    std::set<std::string> filePaths_;
    std::unordered_map<std::string, std::shared_ptr<File>> fileInfoMap_;
    std::unordered_map<std::string, std::set<std::string>> createFileGroupByDir_;

    inline static std::map<std::string, Insight::Scalar::ParseDataType> fileTypeMap_ = {
            {R"(out.tfevent)",        Insight::Scalar::ParseDataType::TF_EVENT},
            {R"(out.events.summary)", Insight::Scalar::ParseDataType::MINDSPORE_SUMMARY},
            {R"(worker_[0-9]+.*\.log)", Insight::Scalar::ParseDataType::TEXT_LOG},
            {R"(npu_.*\.log)",          Insight::Scalar::ParseDataType::TEXT_LOG},
            {R"(rank_.*\.log)",         Insight::Scalar::ParseDataType::TEXT_LOG}
    };
    std::mutex mutex_;
};
}

#endif //FILEINFOMANAGER_H
