/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef FILEWATCH_H
#define FILEWATCH_H

#include <vector>
#include <string>
#include <thread>
#include <shared_mutex>
#include <memory>
#include "defs/ConceptDefs.h"

namespace Insight::Scalar::FileWatch {
class FileWatcher {
public:
    FileWatcher() = default;

    virtual void Init() = 0;

    virtual void AddWatchPath(const std::vector<std::string> &watchFileList) {};

    virtual void DelWatchPath(const std::vector<std::string> &DelFileList) {};

    virtual void OnFileWriteClose(std::string &&dir, std::string &&fileName) {};

    virtual void OnFileCreated(std::string &&dir, std::string &&fileName) {};

    virtual void Reset() = 0;

    virtual ~FileWatcher() = default;
};
}
#endif //FILEWATCH_H
