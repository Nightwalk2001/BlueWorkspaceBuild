/*
 * Copyright (c) , Huawei Technologies Co., Ltd. 2025-2025 .All rights reserved.
 */
#ifndef MINDSTUDIO_SCALAR_NORMALLINE_H
#define MINDSTUDIO_SCALAR_NORMALLINE_H

#include "GraphLine/LineOp/LineOp.h"

namespace Insight::Scalar {

class NormalLineOp : public LineOp {
public:
    NormalLineOp()
    {
        mode_ = DataMode::NORMAL;
        view_ = true;
    }

    ~NormalLineOp() override = default;
    void Process(GraphLine& line) override
    {
        line.GetDataView() = ScalarVisuallyServer::Instance().GetGraphData(tag_, file_, left_, right_, mode_);
    }

    std::shared_ptr<LineOp> Clone() override
    {
        auto res = std::make_shared<NormalLineOp>();
        res->SetDataMode(mode_);
        res->SetBasicInfo(tag_, file_, left_, right_);
        return res;
    }
};
}
#endif //MINDSTUDIO_BOARD_NORMALLINE_H
