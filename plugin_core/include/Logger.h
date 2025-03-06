/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef BOARD_LOGGER_H
#define BOARD_LOGGER_H

#include <unordered_map>
#include <iostream>
#include <string_view>
namespace Insight {
enum class LogRank:int {
    Error= 0,
    Warning,
    Info,
    Exception
};

class Logger {
public:
    static Logger &GetLogger();

    template<typename T>
    Logger &operator<<(const T &val) noexcept
    {
        std::cout << val;
        return *this;
    }

    Logger &operator<<(std::ostream &func(std::ostream &os)) noexcept
    {
        std::cout << func;
        return *this;
    }

    Logger &start(std::string_view rank);
private:
    Logger() = default;
    ~Logger() = default;
};

static inline Logger &LOG(enum Insight::LogRank level)
{
    static std::unordered_map<Insight::LogRank, std::string_view> levelsMap = {
        {LogRank::Error, "Error"},
        {LogRank::Info, "Info"},
        {LogRank::Warning, "Warn"},
        {LogRank::Exception, "Exception"}
    };
    return Logger::GetLogger().start(levelsMap[level]);
}
}
#endif //BOARD_LOGGER_H
