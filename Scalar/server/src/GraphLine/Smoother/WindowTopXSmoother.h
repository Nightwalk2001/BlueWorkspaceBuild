/*
* Copyright (c), Huawei Technologies Co., Ltd. 2025-2025.All rights reserved.
*/

#ifndef WINDOWTOPXSMOOTHER_H
#define WINDOWTOPXSMOOTHER_H
#include <queue>
#include <set>

#include "SmootherBase.h"
namespace Insight::Scalar::Smoothing {
struct WindowToxSmoothingParam: public SmoothingParamBase
{
    WindowToxSmoothingParam()
    {
        algorithm_ = "windowTopx";
    }
    explicit WindowToxSmoothingParam(const json_t& param)
    {
        algorithm_ = "windowTopx";
        if (param.HasMember("windowSize") && param["windowSize"].IsUint64()) {
            windowSize_ = param["windowSize"].GetUint64();
        }
        if (param.HasMember("top") && param["top"].IsDouble()) {
            top_ = param["top"].GetDouble();
        }
    }
    std::unique_ptr<SmoothingParamBase> Clone() override
    {
        auto res = std::make_unique<WindowToxSmoothingParam>();
        res->windowSize_ = this->windowSize_;
        res->top_ = this->top_;
        return res;
    }
    uint64_t windowSize_{0};
    double top_{0.0};
};
class WindowTopXSmoother: public SmootherBase
{
public:
    WindowTopXSmoother()
    {
        algorithm_ = "windowTopx";
    }
    ~WindowTopXSmoother() override = default;
    void SetSampleParam(std::unique_ptr<SmoothingParamBase> param) override;
    void Sample(const std::vector<ScalarPoint>& original, std::vector<ScalarPoint>& dst) override;
    bool Equal(std::unique_ptr<SmootherBase>& other) override;
    void Reset() override;
private:
    uint64_t windowSize_{0};
    double top_{0.0};
    uint64_t elemCount_{0};
    std::multiset<ScalarPoint, ScalarPointCmp> window_;
    using WindowItemIt = std::multiset<ScalarPoint, ScalarPointCmp>::iterator;
    std::queue<WindowItemIt> queue_;

};
}




#endif //WINDOWTOPXSMOOTHER_H
