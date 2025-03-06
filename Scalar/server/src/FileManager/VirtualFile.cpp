/*
* Copyright (c), Huawei Technologies Co., Ltd. 2025-2025.All rights reserved.
 */
#include "VirtualFile.h"
#include <algorithm>
#include "ScalarVisuallyServer.h"
using namespace Insight::Scalar::FileInfo;

void VirtualFile::AddSubFiles(const std::vector<std::string> &files)
{
    if (files.empty()) {
        return;
    }
    for(auto file : files) {
        if (subFiles_.count(file) != 0) {
            continue;
        }
        auto fileInfo = ScalarVisuallyServer::Instance().GetFileInfo(file);
        if (fileInfo == nullptr) {
            continue;
        }
        auto tag = fileInfo->GetContainsTag();
        tag_.insert(tag.begin(), tag.end());
        subFiles_.emplace(std::move(file));
    }
}

std::vector<DataView> VirtualFile::GetData(std::string_view tag, uint64_t left, uint64_t right, DataMode mode)
{
    if (subFiles_.empty()) {
        return {};
    }
    std::vector<DataView> res;
    res.reserve(subFiles_.size());
    std::for_each(subFiles_.begin(), subFiles_.end(), [tag,left, right, mode, &res](const std::string& file){
        auto fileInfo = ScalarVisuallyServer::Instance().GetFileInfo(file);
        if (fileInfo == nullptr || fileInfo->Type() != FileType::NORMAL ||!fileInfo->ContainsData(tag.data())) {
            return;
        }
        auto data = fileInfo->GetData(tag, left, right, mode);
        std::copy(data.begin(), data.end(), std::back_inserter(res));
    });
    // 对数据视图排序, 方便后续处理, 左边界step升序，如果step一致，则按时间戳升序
    std::sort(res.begin(), res.end(), [](const DataView& left, const DataView& right){
        auto l = left.lower();
        auto r = right.lower();
        if (l->step_ != r->step_) {
            return l->step_ < r->step_;
        }
        return l->wallTime_ < r->wallTime_;
    });
    return res;
}

const std::set<std::string>& VirtualFile::GetSubFiles()
{
    return subFiles_;
}

void VirtualFile::UpdateSmoothingParam(std::string_view tag, std::unique_ptr<SmoothingParamBase> smoothingParam)
{
    if (smoothingParam == nullptr) {
        return;
    }
    for (const std::string& file : subFiles_) {
        auto fileInfo = ScalarVisuallyServer::Instance().GetFileInfo(file);
        if (fileInfo == nullptr) {
            continue;
        }
        fileInfo->UpdateSmoothingParam(tag, smoothingParam->Clone());
    }
}




