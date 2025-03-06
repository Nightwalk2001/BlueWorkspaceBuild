/*
* Copyright (c), Huawei Technologies Co., Ltd. 2025-2025.All rights reserved.
 */
#ifndef VIRTUALFILE_H
#define VIRTUALFILE_H
#include <set>
#include "File.h"
namespace Insight::Scalar::FileInfo {
class VirtualFile: public FileInfo::File {
public:
    explicit VirtualFile(std::string fileName): File(std::move(fileName), ParseDataType::TEXT_LOG)
    {
        type_ = FileType::VISUAL;
    }
    ~VirtualFile() override = default;
     void AddSubFiles(const std::vector<std::string> &files) override;
    const std::set<std::string>& GetSubFiles();
    std::vector<DataView> GetData(std::string_view tag, uint64_t left, uint64_t right, DataMode mode) override;
    void UpdateSmoothingParam(std::string_view tag, std::unique_ptr<SmoothingParamBase> smoothingParam) override;
private:
    std::set<std::string> subFiles_;
};
}

#endif //VIRTUALFILE_H
