/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef FILE_H
#define FILE_H
#include <string>
#include <memory>
#include <optional>
#include <mutex>
#include <shared_mutex>
#include <set>

#include "defs/ConceptDefs.h"
#include "Smoother/SmootherFactory.h"

namespace Insight::Scalar::FileInfo {
enum class FileType
{
    NORMAL,
    VISUAL
};


class File
{
public:
    File(std::string filePath, ParseDataType dataType) : filePath_(std::move(filePath)), dataType_(dataType)
    {
    };

    virtual ~File() = default;

    virtual std::vector<DataView> GetDataNormal(std::string_view tag, uint64_t left, uint64_t right, DataMode mode);

    /**
    * @brief 获取文件中某个tag下的数据, 左右边界若存在则都取到，否则向取值范围内取值
    * @param tag: 标签值
    * @param left: 左边界
    * @param right: 右边界
    * @param mode: 获取数据的模式，normal:获取原始数据  smoothing:获取smoothing数据，
    */
    virtual std::vector<DataView> GetData(std::string_view tag, uint64_t left, uint64_t right, DataMode mode);

    void UpdateRange(std::string_view tag, const std::vector<ScalarPoint>& vector);

    /**
    * @brief 更新数据
    */
    virtual void UpdateData(std::string_view tag, std::vector<ScalarPoint>&& points);

    /**
    * @brief 获取文件中某个tag下的smoothing结果数据
    */
    virtual std::vector<DataView> GetSmoothingData(std::string_view tag, uint64_t left, uint64_t right, DataMode mode);


    /**
    * @brief 增加子文件，只能增加或者删除所有
    */
    virtual void AddSubFiles(const std::vector<std::string>& files);

    /**
    * @brief 更新smoother
    */
    virtual void UpdateSmoothingParam(std::string_view tag, std::unique_ptr<SmoothingParamBase> smoothingParam);

    /**
    * @brief 更新token参数，如果是MindForms类型的文件则不会更新seqLength
    */
    void UpdateTokenParam(double globalBatchSize, double seqLength);

    double GetTokenCof() const;


    std::optional<std::tuple<uint64_t, uint64_t>> GetIntersectionRange(std::string_view tag, uint64_t left,
                                                                       uint64_t right);

    /**
    * @brief  二分查找上下边界对应的迭代器，内部封装stl算法
    */
    std::tuple<DataIt, DataIt> GetBoundary(std::vector<ScalarPoint>& data, uint64_t left, uint64_t right);

    void UpdateSmoothingData(std::string_view tag);

    virtual bool ContainsData(std::string_view tag);

    const std::set<std::string>& GetContainsTag();

    FileType Type() const
    {
        return type_;
    }

    void SetType(FileType type)
    {
        type_ = type;
    }

    const std::string& FilePath() const
    {
        return filePath_;
    }

    std::string FilePath()
    {
        return filePath_;
    }

    void SetFilePath(std::string filePath)
    {
        filePath_ = std::move(filePath);
    }

    ParseDataType DataType() const
    {
        return dataType_;
    }

    void SetParseDataType(ParseDataType parseDataType)
    {
        dataType_ = parseDataType;
    }

    uint64_t OffSet() const
    {
        return offSet_;
    }

    void SetOffSet(uint64_t offSet)
    {
        offSet_ = offSet;
    }

    bool Empty() const
    {
        return empty_;
    }

    void SetEmpty(bool empty)
    {
        empty_ = empty;
    }

    bool Imported() const
    {
        return imported_;
    }

    void SetImported(bool imported)
    {
        imported_ = imported;
    }

    double GlobalBatchSize() const
    {
        return globalBatchSize_;
    }

    void SetGlobalBatchSize(double globalBatchSize)
    {
        globalBatchSize_ = globalBatchSize;
    }

    double SeqLength() const
    {
        return seqLength_;
    }

    void SetSeqLength(double seqLength)
    {
        seqLength_ = seqLength;
    }

protected:
    FileType type_{FileType::NORMAL};
    std::unordered_map<std::string, std::vector<ScalarPoint>> data_;
    std::unordered_map<std::string, std::vector<ScalarPoint>> smoothingData_;
    std::unordered_map<std::string, std::shared_ptr<SmootherBase>> smoother_;
    std::unordered_map<std::string, std::tuple<uint64_t, uint64_t>> range_;
    std::shared_mutex dataMutex_; // 控制数据更新的互斥量，这里data_和smoothingData_使用同一个变量，因为更新data_时总需要更新smoothingData_
    std::mutex smootherMutex_; // 控制smoother的更新
    std::set<std::string> tag_;
    std::string filePath_;
    ParseDataType dataType_;
    uint64_t offSet_{0};
    bool empty_{false};
    bool imported_{true};

    // 以下参数用于token模式，mindForms日志只需globalBatchSize参数，其余类型需要globalBatchSize和seqLength
    double globalBatchSize_{-1};
    double seqLength_{-1};
};
}
#endif //FILE_H
