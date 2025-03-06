/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef BOARD_PLUGINS_SCALARVISUALLY_SRC_FILEMANAGER_FILEWATCHERFACTORY_H_
#define BOARD_PLUGINS_SCALARVISUALLY_SRC_FILEMANAGER_FILEWATCHERFACTORY_H_

#include <vector>
#include <string>
#include <thread>
#include <shared_mutex>
#include <memory>
#include "defs/ConceptDefs.h"
#include "FileWatcherLinuxImpl.h"

namespace Insight::Scalar::FileWatch {

class FileWatcherFactory {
public:
    static inline std::unique_ptr<FileWatcher> GetFileWatcher() {
#ifdef __linux__
        return std::make_unique<FileWatcherLinuxImpl>();
#elif defined(_WIN32)
        // TODO windows platform impelement
        return nullptr;
#elif defined(__APPLE__)
        // TODO mac platform impelemnt
        return nullptr;
#endif
        return nullptr;
    }
};
}
#endif //BOARD_PLUGINS_SCALARVISUALLY_SRC_FILEMANAGER_FILEWATCHERFACTORY_H_
