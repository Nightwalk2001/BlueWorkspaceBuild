/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#include "IRLowSmoother.h"
#include <memory>
#include <algorithm>
#include <math.h>

using namespace Insight::Scalar::Smoothing;
constexpr uint64_t getDataLimit = 50000;

void IRLowSmoother::SetSampleParam(std::unique_ptr<SmoothingParamBase> param)
{
    if (param == nullptr) {
        return;
    }
    auto irParam = reinterpret_cast<IRSmootherParam*>(param.get());
    if (irParam->weight_ != weight_) {
        Reset();
        weight_ = irParam->weight_;
    }
}

void IRLowSmoother::Sample(const std::vector<ScalarPoint> &original, std::vector<ScalarPoint> &dst)
{
    if (numAccum_ >= original.size() || weight_ == 0.0) {
        return ;
    }
    float firstValue = original[0].value_;
    bool isConstant = std::all_of(original.begin(), original.end(), [&firstValue](const auto &point) {
        return point.value_ == firstValue;
    });
    for (size_t index = numAccum_; index < original.size(); index++) {
        auto sampledPoint = original[index];
        if (isConstant || std::isinf(sampledPoint.value_) || std::isnan(sampledPoint.value_)) {
            // do nothing
        } else {
            last_ = last_ * weight_ + (1 - weight_) * sampledPoint.value_;
            numAccum_++;
            float debiasWeight = 1.0;
            if (weight_ != 1.0) {
                debiasWeight = debiasWeight - static_cast<float>(pow(weight_, numAccum_));
            }
            sampledPoint.value_ = last_ / debiasWeight;
        }
        dst.emplace_back(sampledPoint);
    }
}

bool IRLowSmoother::Equal(std::unique_ptr<SmootherBase> &other)
{
    if (other == nullptr) {
        return false;
    }
    if (other->GetAlgorithm() != GetAlgorithm()) {
        return false;
    }
    // 相同算法意味着可以进行类型转换
    if (const auto tmp = reinterpret_cast<IRLowSmoother *>(other.get()); tmp->GetWeight() != weight_) {
        return false;
    }
    return true;
}

void IRLowSmoother::Reset()
{
    last_ = 0;
    numAccum_ = 0;
}

