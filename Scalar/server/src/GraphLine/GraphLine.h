/*
* Copyright (c) , Huawei Technologies Co., Ltd. 2025-2025 .All rights reserved.
 */
#ifndef MINDSTUDIO_SCALAR_GRAPHLINE_H
#define MINDSTUDIO_SCALAR_GRAPHLINE_H

#include <string>
#include <memory>
#include <unordered_map>
#include <queue>
#include <tuple>
#include <Util/ScalaryProtocolUtil.h>

namespace Insight::Scalar {
/**
 * @brief line type
 * @attention use priority queue to manager the sequence of line
 */
enum LineType : int {
    NORMAL = 1, SAMPLE = 0, SMOOTHING = 2, TOKEN = 3, NORMAL_SMOOTHING = 4, TOKEN_SMOOTHING = 5, UNKNOWN = -1
};

inline LineType CastLineType(std::string_view type)
{
    static std::unordered_map<std::string_view, LineType> map = {{"normal",    LineType::NORMAL},
                                                                 {"sample",    LineType::SAMPLE},
                                                                 {"smoothing", LineType::SMOOTHING},
                                                                 {"token",     LineType::TOKEN}};
    if (map.find(type) == map.end()) {
        return LineType::UNKNOWN;
    }
    return map[type];
}

inline std::string_view CastLineTypeStr(LineType type)
{
    static std::unordered_map<LineType, std::string_view> map = {{LineType::NORMAL,           "normal"},
                                                                 {LineType::SAMPLE,           "sample"},
                                                                 {LineType::SMOOTHING,        "smoothing"},
                                                                 {LineType::TOKEN,            "token"},
                                                                 {LineType::NORMAL_SMOOTHING, "normalSmoothing"},
                                                                 {LineType::TOKEN_SMOOTHING,  "tokenSmoothing"}};
    if (map.find(type) == map.end()) {
        return "unknown";
    }
    return map[type];
}

struct LineData {
    std::string_view lineType_;
    std::vector<ScalarPoint> data_;
};

struct LineBasicInfo
{
    LineBasicInfo() = default;

    LineBasicInfo(std::string tag, std::string file, uint64_t start, uint64_t end): tag_(std::move(tag)),
        file_(std::move(file)), start_(start), end_(end)
    {
    }

    std::string tag_;
    std::string file_;
    uint64_t start_{0};
    uint64_t end_{0};
};

/**
 * @brief abstract factory
 */
class LineOp;
class GraphLine
{
public:
    friend LineOp;
    GraphLine() = default;

    explicit GraphLine(LineType type) : type_(type)
    {
    };

    LineType GetType() const
    {
        return type_;
    }

    void SetType(LineType type)
    {
        type_ = type;
    }

    bool View() const { return view_; }
    void SetView(bool flag)
    {
        view_ = flag;
    }

    [[nodiscard]] std::string File() const
    {
        return file_;
    }


    std::vector<ScalarPoint> GetLineData();
    void AddLineData(std::vector<ScalarPoint>&& points);

    std::vector<DataView>& GetDataView();

    static std::vector<GraphLine> BuildLine(const std::unordered_map<LineType, std::shared_ptr<LineOp>>& lineOpMap);
private:
    LineType type_{LineType::UNKNOWN};
    std::string tag_;
    std::string file_;
    uint64_t left_{0};
    uint64_t right_{0};
    bool view_{true};
    std::vector<ScalarPoint> srcData_;
    std::vector<DataView> dataView_;
};
}
#endif //MINDSTUDIO_SCALAR_GRAPHLINE_H
