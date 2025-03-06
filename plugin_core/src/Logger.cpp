/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#include <iomanip>
#include <chrono>
#include <ctime>
#include "Logger.h"

namespace Insight {
Logger &Logger::GetLogger()
{
    std::ios::sync_with_stdio(false); // improve the std::cout performance
    static Logger errorLogger;
    return errorLogger;
}

Logger &Logger::start(std::string_view rank)
{
    const time_t current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm local_time{};
#ifdef _WIN32
    localtime_s(&local_time, &current_time);
#else
    localtime_r(&current_time, &local_time);
#endif
    std::ostringstream oss;
    oss << std::put_time(&local_time, "%Y-%m-%d %H-%M-%S");
    *this << std::endl << oss.str() << "|[" << rank << "] " << std::flush;
    return *this;
}
}
