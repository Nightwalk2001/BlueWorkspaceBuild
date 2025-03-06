/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#include "FileInfoManager.h"
#include "Logger.h"

using namespace Insight::Scalar::FileInfo;
using namespace Insight;

std::shared_ptr<File> FileInfoManager::AddFile(const std::string &filePath, ParseDataType dataType)
{
    std::unique_lock lock(mutex_);
    if (filePaths_.count(filePath) != 0) {
        return fileInfoMap_[filePath];
    }
    auto fileInfo = std::make_shared<File>(filePath, dataType);
    filePaths_.insert(filePath);
    fileInfoMap_[filePath] = fileInfo;
    return fileInfo;
}

bool FileInfoManager::DelFileFromMap(std::string_view filePath) {
    std::unique_lock lock(mutex_);
    if (filePaths_.count(filePath.data()) == 0) {
        return false;
    }
    filePaths_.erase(filePath.data());
    fileInfoMap_.erase(filePath.data());
    return true;
}

std::shared_ptr<File> FileInfoManager::GetFileInfo(std::string_view filePath)
{
    std::unique_lock lock(mutex_);
    if (filePaths_.count(filePath.data()) == 0) {
        return nullptr;
    }
    return fileInfoMap_[filePath.data()];
}

void FileInfoManager::Reset() {
    std::unique_lock lock(mutex_);
    filePaths_.clear();
    fileInfoMap_.clear();
}

void FileInfoManager::OnFileCreate(std::string &&dir, std::string &&fileName) {
    std::unique_lock lock(mutex_);
    LOG(LogRank::Error) << "Watched new file under " << dir << ", file:" << fileName;
    createFileGroupByDir_[std::move(dir)].emplace(std::move(fileName));
}

std::unordered_map<std::string, std::set<std::string>> FileInfoManager::GetCreatedFileGroupByDir() {
    std::unique_lock lock(mutex_);
    auto res = std::move(createFileGroupByDir_);
    createFileGroupByDir_ = std::unordered_map<std::string, std::set<std::string>>();
    return res;
}



