/*
* Copyright (c) , Huawei Technologies Co., Ltd. 2025-2025 .All rights reserved.
 */
#ifndef LINEOPFACTORY_H
#define LINEOPFACTORY_H
#include "NormalLineOp.h"
#include "SampleLineOp.h"
#include "SmoothingLineOp.h"
#include "TokenLineOp.h"

namespace Insight::Scalar {
static void BuildLineOps(const json_t& param,
                         const LineBasicInfo& basicInfo,
                         std::unordered_map<LineType, std::shared_ptr<LineOp>>& opsMap)
{
    auto type = CastLineType(param["type"].GetString());
    bool enable = param["enable"].GetBool();
    if (!enable) {
        return;
    }
    std::shared_ptr<LineOp> ops;
    switch (type) {
    case LineType::SMOOTHING:
        {
            ops = std::make_unique<SmoothingLineOp>();
            break;
        }
    case NORMAL:
        {
            ops = std::make_unique<NormalLineOp>();
            break;
        }
    case TOKEN:
        {
            ops = std::make_unique<TokenLineOp>();
            break;
        }
    default:
        break;
    }
    ops->SetBasicInfo(basicInfo.tag_, basicInfo.file_, basicInfo.start_, basicInfo.end_);
    ops->BuildFromJson(param);
    if (ops != nullptr) {
        opsMap.insert({type, ops});
    }
}
}
#endif //LINEOPFACTORY_H
