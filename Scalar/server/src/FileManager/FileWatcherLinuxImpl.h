/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef FILEWATCHLINUXIMPL_CPP_H
#define FILEWATCHLINUXIMPL_CPP_H

#include "FileWatcher.h"
#ifdef __linux__
#include <shared_mutex>
#include <sys/inotify.h>
#include <set>
#include <unordered_map>
#include <condition_variable>
#include <memory>

namespace Insight::Scalar::FileWatch {
class FileWatcherLinuxImpl : public FileWatcher {
public:
    FileWatcherLinuxImpl() = default;

    void Init() override;
    void AddWatchPath(const std::vector<std::string> &watchFileList) override;
    void WaitStopWatchThread(unsigned int millSeconds);
    static void WatchFunc(FileWatcherLinuxImpl &watcher);
    void DelWatchPath(const std::vector<std::string> &DelFileList) override;
    void OnFileCreated(std::string &&dir, std::string &&fileName) override;
    void OnFileWriteClose(std::string &&dir, std::string &&fileName) override;
    void Reset() override;
    bool IsDirWatched(const std::string &dir);
    bool IsDirWatched(int wd);
    bool IsFileWatched(const std::string &dir, const std::string &file);
    std::string GetWatchedDirName(int wd);
    void NotifyWatchThreadStopped();
    ~FileWatcherLinuxImpl() override = default;

    std::unique_ptr<std::thread> watchThread_;
    int inotifyFd_ = -1;
    std::unordered_map<std::string, int> watchDirFd_;
    std::unordered_map<int, std::string> watchFdDir_;
    std::unordered_map<std::string, std::set<std::string>> watchDirToFiles_;
    std::shared_mutex mutex_;
    std::mutex exitMutex_;
    std::condition_variable exitCv;
    bool stop_ = false;
    inline static int watchFlag_ = IN_CLOSE_WRITE | IN_MODIFY | IN_MOVED_TO | IN_CREATE;
};
}
#endif // __linux__

#endif //FILEWATCHLINUXIMPL_CPP_H
