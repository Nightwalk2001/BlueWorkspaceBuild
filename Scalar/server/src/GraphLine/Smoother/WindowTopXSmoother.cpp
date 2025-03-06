/*
* Copyright (c), Huawei Technologies Co., Ltd. 2025-2025.All rights reserved.
*/

#include "WindowTopXSmoother.h"
#include <algorithm>
#include <cmath>
#include <numeric>
using namespace Insight::Scalar::Smoothing;

void WindowTopXSmoother::Reset()
{
    numAccum_ = 0;
    window_.clear();
    elemCount_ = 0;
    queue_ = {};
}

void WindowTopXSmoother::SetSampleParam(std::unique_ptr<SmoothingParamBase> param)
{
    if (param == nullptr) {
        return;
    }
    if (param->algorithm_ != algorithm_) {
        return;
    }
    auto topXParam = reinterpret_cast<WindowToxSmoothingParam*>(param.get());
    if (topXParam->windowSize_ == windowSize_ && topXParam->top_ == top_) {
        return;
    }
    Reset();
    windowSize_ = topXParam->windowSize_;
    top_ = topXParam->top_;
    elemCount_ = std::floor(windowSize_ * top_);
    if (elemCount_ == 0) {
        elemCount_ = 1;
    }
}

bool WindowTopXSmoother::Equal(std::unique_ptr<SmootherBase>& other)
{
    if (other->GetAlgorithm() != algorithm_) {
        return false;
    }
    auto smoother = reinterpret_cast<WindowTopXSmoother*>(other.get());
    if (smoother->windowSize_ != windowSize_) {
        return false;
    }
    if (smoother->top_ != windowSize_) {
        return false;
    }
    return true;
}


void WindowTopXSmoother::Sample(const std::vector<ScalarPoint>& original, std::vector<ScalarPoint>& dst)
{
    if (numAccum_ >= original.size()) {
        return;
    }
    for (uint64_t index  = numAccum_; index < original.size(); index++) {
        const auto& origin = original[index];
        if (windowSize_ == 0) { // 窗口大小为0或头部不足窗口大小, 不做平滑，直接填原始值
            dst.emplace_back(origin);
            numAccum_++;
            continue;
        }
        if (window_.size() < windowSize_ - 1) {
            dst.emplace_back(origin);
            auto it  = window_.insert(origin);
            queue_.push(it);
            numAccum_++;
            continue;
        }
        numAccum_++;
        if (window_.size() == windowSize_) { // 插入后大于窗口大小，弹出最左边的值
            // 1. 弹出原始值
            auto it = queue_.front();
            queue_.pop();
            window_.erase(it);
        }
        auto it  = window_.insert(origin);
        queue_.push(it);

        ScalarPoint result(origin);
        auto begin = window_.begin();
        auto end = begin;
        std::advance(end, elemCount_);
        auto value = std::accumulate(begin, end, 0.0f, []( float res, const ScalarPoint& point) {
            return point.value_ + res;
        });
        result.value_ = value / elemCount_;
        dst.emplace_back(std::move(result));
    }
}


