/*
 * Copyright (c) , Huawei Technologies Co., Ltd. 2025-2025 .All rights reserved.
 */
#ifndef MINDSTUDIO_SCALAR_SMOOTHINGLINEOP_H
#define MINDSTUDIO_SCALAR_SMOOTHINGLINEOP_H

#include "GraphLine/LineOp/LineOp.h"
#include "Smoother/SmootherFactory.h"
#include "ScalarVisuallyServer.h"

using namespace Insight::Scalar::Smoothing;
namespace Insight::Scalar {
class SmoothingLineOp : public LineOp {
public:
    SmoothingLineOp()
    {
        mode_ = DataMode::SMOOTHING;
        view_ = true;
    }

    ~SmoothingLineOp() override = default;

    void BuildFromJson(const Insight::Scalar::json_t &param) override
    {
        param_ = SmootherParamFactory::Instance().BuildSmoothingParam(param);
    }

    void Process(GraphLine& line) override
    {
        // do nothing
        ScalarVisuallyServer::Instance().UpdateSmoothingParam(tag_, file_, std::move(param_));
        line.GetDataView() = ScalarVisuallyServer::Instance().GetGraphData(tag_, file_, left_, right_, mode_);
    }

    std::shared_ptr<LineOp> Clone() override
    {
        auto res = std::make_shared<SmoothingLineOp>();
        res->SetBasicInfo(this->tag_, this->file_, this->left_, this->right_);
        res->SetSmoothingParam(this->algorithm_, this->param_->Clone());
        res->SetDataMode(mode_);
        return res;
    }

    bool Valid() override
    {
        return !file_.empty() && !algorithm_.empty();
    }
    void SetSmoothingParam(std::string algorithm, std::unique_ptr<SmoothingParamBase> param)
    {
        algorithm_ = std::move(algorithm);
        param_ = std::move(param);
    }
private:
    std::string algorithm_;
    std::unique_ptr<SmoothingParamBase> param_;
 };
}
#endif //MINDSTUDIO_BOARD_SMOOTHINGLINEOP_H
