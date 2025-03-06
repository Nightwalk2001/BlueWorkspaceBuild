/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef CONCEPTDEFS_H
#define CONCEPTDEFS_H

#include <strings.h>
#ifdef _WIN32
    #include <filesystem>
    namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include <cstdint>
#include <sstream>
#include <iomanip>      // std::get_time
#include <ctime>        // struct std::tm
#include <algorithm>
#include "rapidjson.h"
#include "document.h"

namespace Insight::Scalar {
using json_t = rapidjson::Value;
using document_t = rapidjson::Document;

enum class ParseDataType
{
    MINDSPORE_SUMMARY = 0, TF_EVENT = 1, TEXT_LOG = 2, Unknown = 3
};

enum ErrCode : int
{
    OK = 0, INVALID_REQUEST_JSON, REQUEST_INVALID_PARAM, INVALID_PATH
};

enum class DataMode
{
    NORMAL,
    SMOOTHING,
    TOKEN,
    SAMPLE,
    TOKEN_NORMAL,
    TOKEN_SMOOTHING
};

struct ScalarPoint
{
    ScalarPoint() = default;

    ScalarPoint(const ScalarPoint& other)
    {
        step_ = other.step_;
        value_ = other.value_;
        wallTime_ = other.wallTime_;
        localTime_ = other.localTime_;
    }

    ScalarPoint(const int64_t step, const float value) : step_(step), value_(value)
    {
    };

    ScalarPoint(const int64_t step, const float value, const double wallTime) : ScalarPoint(step, value)
    {
        wallTime_ = wallTime;
        localTime_ = CastWallTimeToLocalTime(wallTime_);
    };

    ScalarPoint(const int64_t step, const float value, const std::string& localTime) : ScalarPoint(step, value)
    {
        localTime_ = localTime;
        wallTime_ = CastLocalTimeToWallTime(localTime_);
    }

    [[nodiscard]] const std::string& GetLocalTime() const
    {
        return localTime_;
    }

    static std::string CastWallTimeToLocalTime(double wallTime)
    {
        if (wallTime == -1) {
            return "";
        }
        auto seconds = static_cast<time_t>(wallTime);
        int millseconds = static_cast<int>((wallTime - seconds) * 1000);
        std::tm* tm_info = std::gmtime(&seconds);
        std::ostringstream oss;
        oss << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S") << "," << std::setfill('0') << std::setw(3) << millseconds;
        return oss.str();
    }

    static double CastLocalTimeToWallTime(const std::string& localTime)
    {
        std::tm tm_info = {};
        std::istringstream ss(localTime);
        // 解析日期和时间部分 (不使用 std::get_time)
        char dash; // 用于解析分隔符
        ss >> std::setw(4) >> tm_info.tm_year >> dash // 解析年份
            >> std::setw(2) >> tm_info.tm_mon >> dash // 解析月份
            >> std::setw(2) >> tm_info.tm_mday >> std::ws // 解析日，跳过空白字符
            >> std::setw(2) >> tm_info.tm_hour >> dash // 解析小时
            >> std::setw(2) >> tm_info.tm_min >> dash // 解析分钟
            >> std::setw(2) >> tm_info.tm_sec; // 解析秒

        // 处理毫秒部分
        size_t dot_pos = localTime.find(',');
        int milliseconds = 0;
        if (dot_pos != std::string::npos) {
            milliseconds = std::stoi(localTime.substr(dot_pos + 1, 3)); // 获取毫秒部分
        }

        // tm_year 从1900年开始，需要加上1900
        tm_info.tm_year -= 1900;
        // tm_mon 是从0开始的，需要减去1
        tm_info.tm_mon -= 1;

        // 将 tm 结构转换为 time_t
        std::time_t time_seconds = std::mktime(&tm_info);
        if (time_seconds == -1) {
            return 0.0;
        }

        // 将时间戳加上毫秒部分（毫秒转为秒）
        double timestamp = static_cast<double>(time_seconds) + milliseconds / 1000.0;
        return timestamp;
    }

    int64_t step_{0};
    float value_{0};
    double wallTime_{-1};
    std::string localTime_{}; // %Y-%m-%d-%H:%M:%S.%ms
};

/**
 * 数据视图相关定义，为了尽可能减少拷贝与赋值，仅在需要时创建数据副本
 */
enum ViewType
{
    CONTINUES,
    DISCRETE
};

using DataIt = std::vector<ScalarPoint>::iterator;

class DataView
{
public:
    DataView(const DataIt lower, const DataIt upper, std::string file): lower_(lower), upper_(upper),
                                                                        file_(std::move(file))
    {
        length_ = std::distance(lower_, upper_) + 1;
    }

    explicit DataView(std::string file): file_(std::move(file)), length_(0)
    {
    }

    /**
     * @brief 按一定步长从迭代器范围中获取元素
     * @param step  步长
     * @return 剩余步长，作为下一个view的偏移量
     */
    void GatherElement(uint64_t step)
    {
        if (type_ != ViewType::CONTINUES) {
            //离散类型的视图不支持此操作
            return;
        }
        type_ = ViewType::DISCRETE;
        points_.clear();
        auto index = lower_;
        auto remain = length_;
        length_ = 0;
        while (remain > 0 && index != upper_) {
            points_.push_back(index);
            if (remain < step) {
                break;
            }
            else {
                std::advance(index, step);
                remain -= step;
            }
        }
        if (index != upper_) {
            points_.push_back(--upper_);
        }
    }

    const std::vector<DataIt>& GetDatas() const { return points_; }

    std::vector<ScalarPoint> GetElements() const
    {
        std::vector<ScalarPoint> data;
        data.reserve(Length());
        if (type_ == ViewType::CONTINUES) {
            std::transform(lower_, upper_, std::back_inserter(data),
                           [](const auto& point) {
                              return point;
                           });
        }
        else {
            std::for_each(points_.begin(), points_.end(), [&data](const DataIt& it) {
                ScalarPoint point;
                point.step_ = it->step_;
                point.value_ = it->value_;
                point.localTime_ = it->localTime_;
                point.wallTime_ = it->wallTime_;
                data.emplace_back(std::move(point));
            });
        }
        return data;
    }

public:
    [[nodiscard]] std::string file() const
    {
        return file_;
    }

    [[nodiscard]] DataIt lower() const
    {
        return lower_;
    }

    [[nodiscard]] DataIt upper() const
    {
        return upper_;
    }

    [[nodiscard]] ViewType type() const
    {
        return type_;
    }

    [[nodiscard]] uint64_t Length() const { return type_ == ViewType::CONTINUES ? length_ : points_.size(); }

private:
    std::string file_; // 标识数据视图属于哪个文件
    DataIt lower_; // 数据视图的下界
    DataIt upper_; // 数据视图的上界
    ViewType type_{ViewType::CONTINUES}; // 数据视图的类型, CONTINUES:数据视图为下界到上界的连续线， DISCRETE: 数据视图为下界到上界范围内按一定规则选取的离散的点
    uint64_t length_;
    std::vector<DataIt> points_;
};

constexpr int lowerIndex = 0;
constexpr int upperIndex = 1;
constexpr int tokenIndex = 2;
}
#endif //CONCEPTDEFS_H
