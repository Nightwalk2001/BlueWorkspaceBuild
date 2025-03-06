/*
 * Copyright (c) , Huawei Technologies Co., Ltd. 2025-2025 .All rights reserved.
 */
#include "GraphLine.h"
#include <algorithm>
#include "LineOp/SmoothingLineOp.h"
#include "LineOp/NormalLineOp.h"
#include "LineOp/TokenLineOp.h"

using namespace Insight::Scalar::Protocol;

std::vector<Insight::Scalar::ScalarPoint> GraphLine::GetLineData()
{
    if (!view_) {
        return std::move(srcData_);
    }
    view_ = false;
    std::for_each(dataView_.begin(), dataView_.end(), [this](const DataView& view) {
        AddLineData(std::move(view.GetElements()));
    });
    return std::move(srcData_);
}

std::vector<DataView>& GraphLine::GetDataView()
{
    return dataView_;
}

std::vector<GraphLine> GraphLine::BuildLine(const std::unordered_map<LineType, std::shared_ptr<LineOp>>& lineOpMap)
{
    std::vector<GraphLine> res;
    bool tokenEnable = false;
    bool smoothingEnable = false;
    if (lineOpMap.find(LineType::TOKEN) != lineOpMap.end()) {
        tokenEnable = true;
    }
    if (lineOpMap.find(LineType::SMOOTHING) != lineOpMap.end()) {
        smoothingEnable = true;
    }
    if (tokenEnable) {
        res.emplace_back(LineType::TOKEN);
        if (smoothingEnable) {
            res.emplace_back(LineType::TOKEN_SMOOTHING);
        }
    } else {
        res.emplace_back(LineType::NORMAL);
        if (smoothingEnable) {
            res.emplace_back(LineType::NORMAL_SMOOTHING);
        }
    }
    return res;
}

void GraphLine::AddLineData(std::vector<ScalarPoint>&& points)
{
    if (view_ || points.empty()) {
        return;
    }
   // 后添加的文件覆盖前面的数据
    const uint64_t left = points[0].step_;
    const auto it = std::lower_bound(srcData_.begin(), srcData_.end(), left, [](const ScalarPoint& point, uint64_t left) {
        return point.step_ < left;
    });
    srcData_.erase(it, srcData_.end());
    std::move(points.begin(), points.end(), std::back_inserter(srcData_));
}
