/*
 * Copyright (c) , Huawei Technologies Co., Ltd. 2025-2025 .All rights reserved.
 */
#ifndef MINDSTUDIO_SCALAR_LINEOP_H
#define MINDSTUDIO_SCALAR_LINEOP_H

#include <string>
#include <memory>
#include <unordered_map>
#include <queue>
#include "Util/ScalaryProtocolUtil.h"
#include "GraphLine.h"

namespace Insight::Scalar {
class LineOp
{
public:
    virtual ~LineOp() = default;

    using DataIter = std::vector<ScalarPoint>::iterator;

    virtual void BuildFromJson(const json_t& param)
    {
    };

    inline void SetBasicInfo(const std::string& tag, const std::string& file, uint64_t left, uint64_t right)
    {
        file_ = file;
        tag_ = tag;
        left_ = left;
        right_ = right;
    }

    virtual void Process(GraphLine& line)
    {
    }

    virtual std::shared_ptr<LineOp> Clone() = 0;

    virtual bool Valid()
    {
        return true;
    }

    [[nodiscard]] const std::string& File() const { return file_; }
    std::string File() { return file_; }

    [[nodiscard]] const std::string& Tag() const { return tag_; }

    std::string Tag() { return file_; }

    inline void SetDataMode(DataMode mode) { mode_ = mode; }

protected:
    std::string file_;
    std::string tag_;
    uint64_t left_{0};
    uint64_t right_{0};
    DataMode mode_{DataMode::NORMAL};
    DataIter begin_;
    DataIter end_;
    bool view_{false};
};



}
#endif //MINDSTUDIO_BOARD_LINEOP_H
