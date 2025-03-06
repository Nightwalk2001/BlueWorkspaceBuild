/*
 * Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef MINDSTUDIO_BOARD_SMOOTHERFACTORY_H
#define MINDSTUDIO_BOARD_SMOOTHERFACTORY_H


#include <memory>
#include <functional>
#include <unordered_map>
#include <string_view>
#include <string>
#include "SmootherBase.h"
#include "IRLowSmoother.h"
#include "WindowMedianSmoother.h"
#include "WindowTopXSmoother.h"
namespace Insight::Scalar::Smoothing {
class SmootherFactory
{
public:
    static SmootherFactory& Instance()
    {
        static SmootherFactory instance;
        return instance;
    }

    SmootherFactory()
    {
        sampleMap_ = {
            {
                "smoothing", []() {
                    return std::make_unique<IRLowSmoother>();
                }
            },
            {
                "windowMedian", [](){ return std::make_unique<WindowMedianSmoother>();}
            },
            {
                "windowTopx", []() {return std::make_unique<WindowTopXSmoother>();}
            }
        };
    }

    std::unique_ptr<SmootherBase> GetSampler(std::string_view algorithm)
    {
        auto it = sampleMap_.find(algorithm);
        if (it == sampleMap_.end()) {
            return nullptr;
        }
        auto func = it->second;
        return func();
    }

private:
    std::unordered_map<std::string_view, std::function<std::unique_ptr<SmootherBase>()>> sampleMap_{};
};

class SmootherParamFactory
{
public:
    static SmootherParamFactory& Instance()
    {
        static SmootherParamFactory instance;
        return instance;
    }

    std::unique_ptr<SmoothingParamBase> BuildSmoothingParam(const json_t& param)
    {
        std::string algorithm = param["algorithm"].GetString();
        if (map_.find(algorithm) != map_.end()) {
            return map_.at(algorithm)(param);
        }
        return nullptr;
    }

private:
    SmootherParamFactory()
    {
        map_ = {
            {"smoothing", [](const json_t& param) { return std::make_unique<IRSmootherParam>(param); }},
            {"windowMedian", [](const json_t& param) {return std::make_unique<WindowMedianParam>(param);}},
            {"windowTopx", [](const json_t& param) {return std::make_unique<WindowToxSmoothingParam>(param);}}
        };
    }

    ~SmootherParamFactory() = default;

    std::unordered_map<std::string_view, std::function<std::unique_ptr<SmoothingParamBase>(const json_t&)>> map_;
};
}
#endif //MINDSTUDIO_BOARD_SMOOTHERFACTORY_H
