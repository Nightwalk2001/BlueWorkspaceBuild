/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#include "File.h"
#include <climits>
#include <algorithm>
#include <cmath>
using namespace Insight::Scalar::FileInfo;

/**
 * @brief 从数据集中获取step的范围，此函数仅此CPP可见
 * @param data
 * @return
 */
std::tuple<uint64_t, uint64_t> GetRangeFromData(std::vector<ScalarPoint> data)
{
    if (data.empty()) {
        return {std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max()};
    }
    return {data.front().step_, data.back().step_};
}

void File::UpdateTokenParam(double globalBatchSize, double seqLength)
{
    if (globalBatchSize > 0) {
        globalBatchSize_ = globalBatchSize;
    }
    // MindForms框架的text日志中 seqLength从文件中解析，不需要设置
    if (seqLength > 0 && dataType_ != ParseDataType::TEXT_LOG) {
        seqLength_ = seqLength;
    }
}

void File::UpdateSmoothingParam(const std::string_view tag, std::unique_ptr<SmoothingParamBase> smoothingParam)
{
    std::unique_lock lock(smootherMutex_);
    if (smoothingParam == nullptr) {
        return;
    }
    auto smoother = SmootherFactory::Instance().GetSampler(smoothingParam->algorithm_);
    smoother->SetSampleParam(std::move(smoothingParam));
    const auto it = smoother_.find(tag.data());
    if (it == smoother_.end()) {
        smoother_[tag.data()] = std::move(smoother);
        UpdateSmoothingData(tag);
    }
    else if (!it->second->Equal(smoother)) {
        it->second = std::move(smoother);
        UpdateSmoothingData(tag);
    }
}

void File::UpdateData(std::string_view tag, std::vector<ScalarPoint>&& points)
{
    if (points.empty()) {
        return;
    }
    {
        // 避免死锁
        std::unique_lock lock(dataMutex_);
        tag_.insert(tag.data());
        UpdateRange(tag, points);
        std::move(points.begin(), points.end(), std::back_inserter(data_[tag.data()]));
    }
    std::unique_lock lock(smootherMutex_);
    if (smoother_.find(tag.data()) == smoother_.end()) {
        return;
    }
    auto smoother = smoother_.at(tag.data());
    smoother->Sample(data_.at(tag.data()), smoothingData_[tag.data()]);
}

std::vector<DataView> File::GetData(std::string_view tag, uint64_t left, uint64_t right, DataMode mode)
{
    if (tag_.count(tag.data()) == 0) {
        return {};
    }
    if (mode == DataMode::TOKEN_NORMAL || mode == DataMode::TOKEN_SMOOTHING) {
        left = std::floor(static_cast<long double>(left) / GetTokenCof());
        right = std::ceil(static_cast<long double>(right) / GetTokenCof());
    }
    const auto interRange = GetIntersectionRange(tag, left, right);
    if (interRange == std::nullopt) {
        return {};
    }
    std::tie(left, right) = interRange.value();
    switch (mode) {
    case DataMode::TOKEN: // Token与Normal是同一数据源，只是对step做了处理
        return GetDataNormal(tag, left, right, DataMode::TOKEN);
    case DataMode::TOKEN_NORMAL:
    case DataMode::NORMAL:
        return GetDataNormal(tag, left, right, DataMode::NORMAL);
    case DataMode::TOKEN_SMOOTHING:
    case DataMode::SMOOTHING:
        return GetSmoothingData(tag, left, right, DataMode::SMOOTHING);
    default:
        return {};
    }
}

std::vector<DataView> File::GetDataNormal(std::string_view tag, uint64_t left, uint64_t right, DataMode mode)
{
    auto& data = data_.at(tag.data());
    if (data.empty()) {
        return {};
    }
    auto [lower, upper] = GetBoundary(data, left, right);
    DataView view(lower, upper, filePath_);
    return {view};
}

std::optional<std::tuple<uint64_t, uint64_t>> File::GetIntersectionRange(
    std::string_view tag, uint64_t left, uint64_t right)
{
    if (range_.find(tag.data()) == range_.end()) {
        return std::nullopt;
    }
    auto [leftCur, rightCur] = range_.at(tag.data());
    if (left > rightCur || right < leftCur) {
        return std::nullopt;
    }
    auto resLeft = std::max(left, leftCur);
    auto resRight = std::min(right, rightCur);
    return std::tuple<uint64_t, uint64_t>{resLeft, resRight};
}


void File::UpdateRange(std::string_view tag, const std::vector<ScalarPoint>& vector)
{
    if (vector.empty()) {
        return;
    }
    auto range = GetRangeFromData(vector);
    if (std::get<0>(range) == std::numeric_limits<uint64_t>::max()) {
        // means invalid
        return;
    }
    const auto oldRange = range_.find(tag.data());
    if (oldRange == range_.end()) {
        range_[tag.data()] = range;
        return;
    }
    // map[tag] = {map[tag][2], right}的写法会导致崩溃
    uint64_t left = std::get<0>(oldRange->second);
    uint64_t right = std::get<1>(range);
    range_[tag.data()] = {left, right};
}

std::vector<DataView> File::GetSmoothingData(std::string_view tag, uint64_t left, uint64_t right, DataMode mode)
{
    if (smoother_.find(tag.data()) == smoother_.end()) {
        return {};
    }
    if (smoothingData_.find(tag.data()) == smoothingData_.end()) {
        return {};
    }
    auto& data = smoothingData_.at(tag.data());
    if (data.empty()) {
        return {};
    }
    auto [lower, upper] = GetBoundary(data, left, right);
    return {DataView(lower, upper, filePath_)};
}

void File::AddSubFiles(const std::vector<std::string>& files)
{
    // 基类中不做任何操作
}

std::tuple<DataIt, DataIt> File::GetBoundary(std::vector<ScalarPoint>& data, const uint64_t left, const uint64_t right)
{
    // 隐含条件:data已升序排列
    auto lower = std::lower_bound(data.begin(), data.end(), left, [](const ScalarPoint& point, uint64_t value) {
        return point.step_ < value;
    });
    auto upper = std::upper_bound(data.begin(), data.end(), right, [](uint64_t value, const ScalarPoint& point) {
        return value < point.step_;
    });
    // if (upper != data.begin()) {
    //     upper--;
    // }
    return {lower, upper};
}

double File::GetTokenCof() const
{
    constexpr double cof = 0.001; // 单位转换 Billion->million
    if (globalBatchSize_ < 0 || seqLength_ < 0) {
        return 1.0;
    }
    return globalBatchSize_ * seqLength_ * cof;
}

void File::UpdateSmoothingData(std::string_view tag)
{
    // 这里对于smoother的加锁放在函数外，因为函数不会被单独调用
    std::unique_lock lock(dataMutex_);
    auto smoother = smoother_[tag.data()];
    smoother->Sample(data_[tag.data()], smoothingData_[tag.data()]);
}

bool File::ContainsData(std::string_view tag)
{
    return data_.find(tag.data()) != data_.end();
}

const std::set<std::string>& File::GetContainsTag()
{
    return tag_;
}
