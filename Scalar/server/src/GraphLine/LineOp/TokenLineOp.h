/*
 * Copyright (c) , Huawei Technologies Co., Ltd. 2025-2025 .All rights reserved.
 */
#ifndef MINDSTUDIO_SCALAR_TOKENLINE_H
#define MINDSTUDIO_SCALAR_TOKENLINE_H

#include <functional>
#include "GraphLine/LineOp/LineOp.h"
#include "ScalarVisuallyServer.h"

namespace Insight::Scalar {
class TokenLineOp : public LineOp
{
public:
    TokenLineOp()
    {
        mode_ = DataMode::TOKEN;
        view_ = false;
    }

    ~TokenLineOp() override = default;

    void Process(GraphLine& line) override
    {
        if (line.View()) {
            // 此时GraphLine的数据还是View
            TransViewToData(line);
        }
    }

    static void TransViewToData(GraphLine& line)
    {
        line.SetView(false);
        auto tokenTrans = [](const DataIt& it, double tokenCof) {
            ScalarPoint point(*it);
            point.step_ = static_cast<int64_t>(static_cast<double>(point.step_) * tokenCof);
            return point;
        };
        std::for_each(line.GetDataView().begin(), line.GetDataView().end(), [&line, &tokenTrans](const DataView& view) {
            auto fileInfo = ScalarVisuallyServer::Instance().GetFileInfo(view.file());
            if (fileInfo == nullptr) {
                return;
            }
            double tokenCof = fileInfo->GetTokenCof();
            if (tokenCof <= 0) {
                tokenCof = 1.0;
            }
            std::vector<ScalarPoint> data = view.GetElements();
            std::for_each(data.begin(), data.end(), [&tokenCof](ScalarPoint& point) {
                point.step_ = static_cast<int64_t>(tokenCof * static_cast<double>(point.step_));
            });
            line.AddLineData(std::move(data));
        });
    }

    std::shared_ptr<LineOp> Clone() override
    {
        auto res = std::make_shared<TokenLineOp>();
        res->SetBasicInfo(this->tag_, this->file_, this->left_, this->right_);
        return res;
    }
};
}
#endif //MINDSTUDIO_SCALAR_TOKENLINE_H
