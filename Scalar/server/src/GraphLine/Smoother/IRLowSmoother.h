/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#ifndef MINDSTUDIO_BOARD_PLUGINS_MINDSTUDIO_INSIGHT_PLUGINS_SCALAR_SERVER_SRC_SAMPLER_IRLOWPASSSAMPLER_H_
#define MINDSTUDIO_BOARD_PLUGINS_MINDSTUDIO_INSIGHT_PLUGINS_SCALAR_SERVER_SRC_SAMPLER_IRLOWPASSSAMPLER_H_

#include "SmootherBase.h"
#include <unordered_map>

using namespace Insight::Scalar::Smoothing;
struct IRSmootherParam: public SmoothingParamBase
{
    IRSmootherParam()
    {
        algorithm_ = "smoothing";
    }
    explicit IRSmootherParam(const json_t& param)
    {
        algorithm_ = "smoothing";
        if (param.HasMember("weight") && param["weight"].IsFloat()) {
            weight_ = param["weight"].GetFloat();
        }
    }
    std::unique_ptr<SmoothingParamBase> Clone() override
    {
        auto res = std::make_unique<IRSmootherParam>();
        res->weight_ = this->weight_;
        return res;
    }

    float weight_{0.0};
};
class IRLowSmoother : public SmootherBase {
public:
    IRLowSmoother()
    {
        algorithm_ = "smoothing";
    };

    void SetSampleParam(std::unique_ptr<SmoothingParamBase> param) override;

    void Sample(const std::vector<ScalarPoint> &original, std::vector<ScalarPoint> &dst) override;

    bool Equal(std::unique_ptr<SmootherBase> &other) override;

    void Reset() override;
    inline float GetWeight() const { return weight_; }
    inline float GetLast() const { return last_; }
private:
    float weight_{0.0};
    float last_{0.0};
};

#endif //MINDSTUDIO_BOARD_PLUGINS_MINDSTUDIO_INSIGHT_PLUGINS_SCALAR_SERVER_SRC_SAMPLER_IRLOWPASSSAMPLER_H_
