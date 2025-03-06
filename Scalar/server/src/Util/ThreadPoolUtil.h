/*
 * Copyright (c) , Huawei Technologies Co., Ltd. 2024-2024 .All rights reserved.
 */
#ifndef MINDSTUDIO_BOARD_THREADPOOLUTIL_H
#define MINDSTUDIO_BOARD_THREADPOOLUTIL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

namespace Dic::Core {
class ThreadExecutor {
public:
    inline explicit ThreadExecutor(size_t size) : workerCount_(size), stop_(false)
    {
        for (size_t i = 0; i < workerCount_; i++) {
            workers_.emplace_back([this]() {
                for (;;) {
                    std::function<void()> task;
                    // get lock
                    {
                        std::unique_lock lock(queueMutex_);
                        this->condition_.wait(lock, [this]() { return this->stop_ || !this->tasks_.empty(); });
                        if (this->stop_ || this->tasks_.empty()) {
                            return;
                        }
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    template<class F, class... Args>
    auto Submit(F &&f, Args &&... args) -> std::future<typename std::result_of_t<F(Args ...)>>
    {
        using Rtype = typename std::result_of_t<F(Args...)>;
        auto task = std::make_shared<std::packaged_task<Rtype()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<Rtype> future = task->get_future();
        {
            std::unique_lock lock(queueMutex_);
            if (stop_) {
                throw std::runtime_error("submit to stopped thread");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        condition_.notify_one();
        return future;
    }

    void Clear()
    {
        std::unique_lock lock(queueMutex_);
        tasks_ = std::queue<std::function<void()>>();
    }


    ~ThreadExecutor()
    {
        {
            std::unique_lock lock(queueMutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (std::thread &worker: workers_)
            worker.join();
    }

private:
    uint64_t workerCount_{0};
    bool stop_{false};
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
};
}
#endif //MINDSTUDIO_BOARD_THREADPOOLUTIL_H
