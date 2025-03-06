/*
* Copyright (c), Huawei Technologies Co., Ltd. 2025-2025.All rights reserved.
*/
#include "WindowMedianSmoother.h"
using namespace Insight::Scalar::Smoothing;

void WindowMedianSmoother::SetSampleParam(std::unique_ptr<SmoothingParamBase> param)
{
    if (param == nullptr) {
        return;
    }
    const auto medianParam = reinterpret_cast<WindowMedianParam*>(param.get());
    if (medianParam->windowSize_ != windowSize_) {
        Reset();
        windowSize_ = medianParam->windowSize_;
        even_ = windowSize_ % 2 == 0;
    }
}

void WindowMedianSmoother::Reset()
{
    numAccum_ = 0;
    window_.clear();
    queue_ = {};
}

bool WindowMedianSmoother::Equal(std::unique_ptr<SmootherBase>& other)
{
    if (other == nullptr) {
        return false;
    }
    if (other->GetAlgorithm() != algorithm_) {
        return false;
    }
    const auto smoother = reinterpret_cast<WindowMedianSmoother*>(other.get());
    return smoother->WindowSize() == windowSize_;
}

void WindowMedianSmoother::Sample(const std::vector<ScalarPoint>& original, std::vector<ScalarPoint>& dst)
{
    if (numAccum_ >= original.size()) {
        return;
    }
    for (uint64_t index  = numAccum_; index < original.size(); index++) {
        const auto& origin = original[index];
        if (windowSize_ == 0 ) { // 窗口大小为0或头部不足窗口的部分, 不做平滑，直接填原始值
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
        auto mid = window_.begin();
        std::advance(mid, static_cast<uint64_t>(windowSize_ / 2));
        if (even_) { //偶数窗口大小
            auto midLow = mid;
            std::advance(midLow, -1);
            result.value_ = (mid->value_ + midLow->value_) / 2.0f;
        } else {
            result.value_ = mid->value_;
        }
        dst.emplace_back(std::move(result));
    }
}

