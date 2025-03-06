/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#include "FileWatcherLinuxImpl.h"
#ifdef __linux__
#include <cstring>
#include <experimental/filesystem>
#include <cerrno>
#include <unistd.h>
#include <climits>
#include "Logger.h"
#include "plugin/ScalarVisuallyServer.h"

using namespace Insight::Scalar::FileWatch;
using namespace Insight;
namespace fs = std::experimental::filesystem;
constexpr size_t BUFFER_SIZE = sizeof(struct inotify_event) + NAME_MAX + 1;

void FileWatcherLinuxImpl::Init() {
    inotifyFd_ = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (inotifyFd_ == -1) {
        LOG(LogRank::Error) << "Init file watch, error:" << strerror(errno);
        return;
    }
    watchThread_ = std::make_unique<std::thread>(WatchFunc, std::ref(*this));
    watchThread_->detach();
}

void FileWatcherLinuxImpl::AddWatchPath(const std::vector<std::string> &watchFileList) {
    if (watchFileList.empty()) {
        return;
    }
    for (const auto &file: watchFileList) {
        fs::path path(file);
        std::string dir = path.parent_path();
        std::string fileName = path.filename();
        if (IsFileWatched(dir, fileName)) {
            LOG(LogRank::Info) << "Already watched, not add repeatedly";
            continue;
        }
        int fd = inotify_add_watch(inotifyFd_, dir.c_str(), watchFlag_);
        if (fd == -1) {
            LOG(LogRank::Error) << "Add File Watch failed, error=" << strerror(errno) << ", path=" << file;
            continue;
        }
        std::unique_lock lock(mutex_);
        watchDirFd_.emplace(dir, fd);
        watchFdDir_.emplace(fd, dir);
        watchDirToFiles_[dir].insert(fileName);
    }
}

void FileWatcherLinuxImpl::WaitStopWatchThread(unsigned int millSeconds) {
    {
        std::lock_guard lock(mutex_);
        stop_ = true;
    }
    std::unique_lock<std::mutex> lock(exitMutex_);
    exitCv.wait_for(lock, std::chrono::milliseconds(millSeconds));
    close(inotifyFd_);
}

void FileWatcherLinuxImpl::DelWatchPath(const std::vector<std::string> &DelFileList) {
    if (DelFileList.empty()) {
        LOG(LogRank::Error) << "Del watch failed, path empty";
        return;
    }
    for (const auto &file: DelFileList) {
        fs::path path(file);
        std::string dir = path.root_directory();
        std::string fileName = path.filename();
        if (!IsFileWatched(dir, fileName)) {
            LOG(LogRank::Info) << "Never watch this file";
            continue;
        }
        std::lock_guard lock(mutex_);
        int fd = watchDirFd_[dir];
        if (inotify_rm_watch(inotifyFd_, fd) == -1) {
            LOG(LogRank::Error) << "Delete failed, error=" << strerror(errno);
            continue;
        }
        watchDirToFiles_[dir].erase(fileName);
        if (watchDirToFiles_[dir].empty()) {
            watchDirFd_.erase(dir);
            watchFdDir_.erase(fd);
            watchDirToFiles_.erase(dir);
        }
    }
}

void FileWatcherLinuxImpl::NotifyWatchThreadStopped() {

    std::unique_lock<std::mutex> lock(exitMutex_);
    exitCv.notify_all();
}

void FileWatcherLinuxImpl::WatchFunc(Insight::Scalar::FileWatch::FileWatcherLinuxImpl &watcher) {
    char eventBuff[BUFFER_SIZE] = {0};
    struct inotify_event *event = nullptr;
    while (!watcher.stop_) {
        // read the inotify message
        memset(eventBuff, 0, sizeof(eventBuff));
        ssize_t result = read(watcher.inotifyFd_, eventBuff, sizeof(eventBuff));
        if (result <= 0) {
            continue;
        }
        event = reinterpret_cast<struct inotify_event *>(eventBuff);
        if (!watcher.IsDirWatched(event->wd)) {
            continue;
        }
        std::string dir = watcher.GetWatchedDirName(event->wd);
        std::string fileName = std::string(event->name);
        if ((event->mask & IN_MODIFY) || (event->mask & IN_CLOSE_WRITE)) {
            watcher.OnFileWriteClose(std::move(dir), std::move(fileName));
        } else if ((event->mask & IN_CREATE) || (event->mask & IN_MOVED_TO)) {
            watcher.OnFileCreated(std::move(dir), std::move(fileName));
        }
    }
    LOG(LogRank::Info) << "Exit watch thread";
    watcher.NotifyWatchThreadStopped();
}

void FileWatcherLinuxImpl::OnFileCreated(std::string &&dir, std::string &&fileName) {
    auto &server = ScalarVisuallyServer::Instance();
    LOG(LogRank::Info) << "Find new file in dir:" << dir << ", filename:" << fileName;
    auto fileInfo = server.AddFile(dir + "/" + fileName);
    fileInfo->SetEmpty(true);
    fileInfo->SetImported(false);
    server.OnFileCreate(std::move(dir), std::move(fileName));
}

void FileWatcherLinuxImpl::OnFileWriteClose(std::string &&dir, std::string &&fileName) {
    auto &server = ScalarVisuallyServer::Instance();
    if (!server.IsFileImported(dir + "/" + fileName)) {
        return;
    }
    server.OnFileDataUpdate(std::move(dir), std::move(fileName));
}

void FileWatcherLinuxImpl::Reset() {
    std::lock_guard lock(mutex_);
    for (auto &[path, fd]: watchDirFd_) {
        inotify_rm_watch(inotifyFd_, fd);
    }
    watchDirFd_.clear();
    watchFdDir_.clear();
}

bool FileWatcherLinuxImpl::IsDirWatched(const std::string &dir) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return watchDirFd_.count(dir) != 0;
}

bool FileWatcherLinuxImpl::IsFileWatched(const std::string &dir, const std::string &file) {
    if (!IsDirWatched(dir)) {
        return false;
    }
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return watchDirToFiles_[dir].count(file) != 0;
}

bool FileWatcherLinuxImpl::IsDirWatched(int wd) {
    if (wd == -1) {
        return false;
    }
    std::shared_lock lock(mutex_);
    if (watchFdDir_.count(wd) == 0) {
        return false;
    }
    return true;
}

std::string FileWatcherLinuxImpl::GetWatchedDirName(int wd) {
    if (wd == -1) {
        return "";
    }
    std::shared_lock lock(mutex_);
    if (watchFdDir_.count(wd) == 0) {
        return "";
    }
    return watchFdDir_.at(wd);
}

#endif
