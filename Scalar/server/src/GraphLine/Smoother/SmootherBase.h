/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#ifndef MINDSTUDIO_BOARD_PLUGINS_MINDSTUDIO_INSIGHT_PLUGINS_SCALAR_SERVER_SRC_SAMPLER_SAMPLERBASE_H_
#define MINDSTUDIO_BOARD_PLUGINS_MINDSTUDIO_INSIGHT_PLUGINS_SCALAR_SERVER_SRC_SAMPLER_SAMPLERBASE_H_

#include <memory>
#include <vector>
#include "defs/ConceptDefs.h"

using namespace Insight::Scalar;
namespace Insight::Scalar::Smoothing {
struct ScalarPointCmp
{
    bool operator() (const ScalarPoint& left, const ScalarPoint& right) const
    {
        return left.value_ < right.value_;
    }
};
struct  SmoothingParamBase {
    SmoothingParamBase() = default;

    explicit SmoothingParamBase(std::string algorithm) : algorithm_(std::move(algorithm))
    {}
    explicit SmoothingParamBase(json_t&) {};

    virtual std::unique_ptr<SmoothingParamBase> Clone() { return std::make_unique<SmoothingParamBase>();}
    std::string algorithm_;
};

/**
 * @brief use for data sampler
 */
class SmootherBase {
public:
    virtual std::string GetAlgorithm()
    { return algorithm_; }

    virtual ~SmootherBase() = default;

    virtual void SetSampleParam(std::unique_ptr<SmoothingParamBase> param) = 0;

    virtual void Sample(const std::vector<ScalarPoint> &original, std::vector<ScalarPoint> &dst) = 0;

    virtual bool Equal(std::unique_ptr<SmootherBase> &other) = 0;

    virtual void Reset() = 0;

protected:
    std::string algorithm_;
    uint32_t numAccum_{0};
};
}
#endif //MINDSTUDIO_BOARD_PLUGINS_MINDSTUDIO_INSIGHT_PLUGINS_SCALAR_SERVER_SRC_SAMPLER_SAMPLERBASE_H_
