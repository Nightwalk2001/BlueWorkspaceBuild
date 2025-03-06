/*
* Copyright (c) , Huawei Technologies Co., Ltd. 2025-2025 .All rights reserved.
 */
#ifndef SAMPLELINEOP_H
#define SAMPLELINEOP_H
#include <numeric>

#include "LineOp.h"
#include "ScalarVisuallyServer.h"

namespace Insight::Scalar {
class SampleLineOp : public LineOp
{
public:
    SampleLineOp()
    {
        mode_ = DataMode::SAMPLE;
    }

    ~SampleLineOp() override = default;

    void Process(GraphLine& line) override
    {
       auto& dataView = line.GetDataView();
        if (dataView.empty()) {
            return;
        }
        // 计算步长
        uint64_t total = std::accumulate(dataView.begin(), dataView.end(), 0ull,
                                         [](uint64_t count, const DataView& view) {
                                             return count + std::distance(view.lower(), view.upper());
                                         });
        uint64_t step = total /  maxDataCount;
        if (step == 0) {    // step 至少为1
            step = 1;
        }
        // 抽样，左右边界需要取到
        std::for_each(dataView.begin(), dataView.end(), [step](DataView& view) {
           view.GatherElement(step);
        });
    }

    std::shared_ptr<LineOp> Clone() override
    {
        auto op = std::make_shared<SampleLineOp>();
        op->SetBasicInfo(tag_, file_, left_, right_);
        return op;
    }

protected:
    constexpr static uint64_t maxDataCount = 10001; // 抽样后的最大点的数量, 这里多加1是为了便于计算
};
}


#endif //SAMPLELINEOP_H
