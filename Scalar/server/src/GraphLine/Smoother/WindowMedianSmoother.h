/*
* Copyright (c), Huawei Technologies Co., Ltd. 2025-2025.All rights reserved.
*/
#ifndef WINDOWMEDIANSMOOTHER_H
#define WINDOWMEDIANSMOOTHER_H
#include <queue>
#include <set>
#include "SmootherBase.h"
namespace Insight::Scalar::Smoothing {
struct WindowMedianParam: public SmoothingParamBase
{
    WindowMedianParam()
    {
        algorithm_  = "windowMedian";
    }
    explicit WindowMedianParam(const json_t& param)
    {
        if (param.HasMember("windowSize") && param["windowSize"].IsUint64()) {
            windowSize_ = param["windowSize"].GetUint64();
        }
        algorithm_ = "windowMedian";
    }
    std::unique_ptr<SmoothingParamBase> Clone() override
    {
        auto res = std::make_unique<WindowMedianParam>();
        res->windowSize_ = this->windowSize_;
        return res;
    }
    uint64_t windowSize_{0};
};
class WindowMedianSmoother: public SmootherBase {
public:
    WindowMedianSmoother()
    {
        algorithm_ = "windowMedian";
    }
    ~WindowMedianSmoother() override = default;
    void SetSampleParam(std::unique_ptr<SmoothingParamBase> param) override;
    void Sample(const std::vector<ScalarPoint>& original, std::vector<ScalarPoint>& dst) override;
    bool Equal(std::unique_ptr<SmootherBase>& other) override;
    void Reset() override;
    uint64_t WindowSize() const { return windowSize_; }
private:
    uint64_t windowSize_{0};
    std::multiset<ScalarPoint, ScalarPointCmp> window_;
    using WindowItemIt = std::multiset<ScalarPoint, ScalarPointCmp>::iterator;
    std::queue<WindowItemIt> queue_;
    bool even_{false};
};

}




#endif //WINDOWMEDIANSMOOTHER_H
