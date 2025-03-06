/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef GRAPHMANAGER_H
#define GRAPHMANAGER_H

#include <unordered_map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <set>
#include "defs/ConceptDefs.h"
#include "Smoother/SmootherBase.h"
#include "Util/ScalaryProtocolUtil.h"

using namespace Insight::Scalar::Smoothing;
namespace Insight::Scalar::GraphOp {
using namespace Protocol;

struct GraphData {
    std::string tag_;
    std::string filePath_;
    std::vector<ScalarPoint> lineData_;
};

class Graph {
public:
    Graph() = default;
    /**
     * @brief constructor of Graph, the graphId and tag is need
     * @param graphId
     * @param tag
     */
    explicit Graph(std::string tag) : tag_(std::move(tag)) {
    };
    void UpdateData(const std::string &file, std::vector<ScalarPoint> &&data);

    std::vector<DataView> GetFileData(const std::string &file, uint64_t left, uint64_t right, DataMode mode);

    void UpdateSmoothingParam(const std::string &file, std::unique_ptr<Smoothing::SmoothingParamBase> sampleParam);

    std::vector<std::string> GetDataFiles();
    bool InnerFile(const std::string &file);

    void AddFile(const std::string& file);

    void DelFile(const std::string& file);
private:
    std::string tag_;
    std::set<std::string> dataFiles_;
};

class GraphManager {
public:
    /**
     * @brief
     * @param tag
     * @param file
     * @param left
     * @param right
     * @param mode
     * @return
     */
    std::vector<DataView> GetGraphData(const std::string &tag, const std::string &file, uint64_t left, uint64_t right,
                                       DataMode mode);

    void
    UpdateGraphSmoothingParam(const std::string &tag, const std::string &file, std::unique_ptr<SmoothingParamBase> param);
    void UpdateGraphData(const std::string &tag, const std::string &file, std::vector<ScalarPoint> &&data);
    std::shared_ptr<Graph> GetGraph(const std::string &tag);
    void Reset();
    std::unordered_map<std::string, std::vector<std::string>> GetAllGraphInfo();
    void GetFileTags(std::string &path, std::set<std::string> &tags);
private:
    bool GraphExits(const std::string &tag);

    std::unordered_map<std::string, std::shared_ptr<Graph>> graphs_;
};
}
#endif //GRAPHMANAGER_H
