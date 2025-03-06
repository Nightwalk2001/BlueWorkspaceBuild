/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#include <algorithm>
#include <optional>
#include "Logger.h"
#include "Smoother/SmootherFactory.h"
#include "Util/ScalaryProtocolUtil.h"
#include "GraphManager.h"
#include <ScalarVisuallyServer.h>

using namespace Insight::Scalar::GraphOp;
using namespace Insight;
using namespace Insight::Scalar;
using namespace Protocol;

constexpr uint64_t MAX_POINT_PER_REQUEST = 50000; // avoid frontend performance problem, not pull data in one request
std::vector<DataView> Graph::GetFileData(const std::string &file, uint64_t left, uint64_t right, DataMode mode)
{
    if (!InnerFile(file)) {
        return {};
    }
    auto fileInfo = ScalarVisuallyServer::Instance().GetFileInfo(file);
    if (fileInfo == nullptr) {
        return {};
    }
    return fileInfo->GetData(tag_, left, right, mode);
}

void Graph::UpdateData(const std::string &file, std::vector<ScalarPoint> &&data)
{
    if (!InnerFile(file)) {
        dataFiles_.insert(file);
    }
    if (data.empty()) {
        return;
    }
    auto fileInfo = ScalarVisuallyServer::Instance().GetFileInfo(file);
    if (fileInfo == nullptr || fileInfo->Type() != FileInfo::FileType::NORMAL) {
        return;
    }
    fileInfo->UpdateData(tag_, std::move(data));
}

bool Graph::InnerFile(const std::string &file)
{
    return dataFiles_.count(file) != 0;
}

std::vector<std::string> Graph::GetDataFiles()
{
    std::vector<std::string> res;
    std::copy(dataFiles_.begin(), dataFiles_.end(), std::back_inserter(res));
    return res;
}

void Graph::UpdateSmoothingParam(const std::string &file, std::unique_ptr<Smoothing::SmoothingParamBase> sampleParam)
{
    if (sampleParam == nullptr || sampleParam->algorithm_.empty()) {
        return;
    }
    auto fileInfo = ScalarVisuallyServer::Instance().GetFileInfo(file);
    if (fileInfo == nullptr) {
        return;
    }
    fileInfo->UpdateSmoothingParam(tag_, std::move(sampleParam));

}

void Graph::AddFile(const std::string& file)
{
    dataFiles_.insert(file);
}

void Graph::DelFile(const std::string& file)
{
    dataFiles_.erase(file);
}

std::vector<DataView> GraphManager::GetGraphData(const std::string &tag, const std::string &file, uint64_t left,
                                                 uint64_t right, DataMode mode)
{
    if (!GraphExits(tag)) {
        LOG(LogRank::Error) << "Get data failed, no such graph, tag:" << tag;
        return {};
    }
    auto graph = GetGraph(tag);
    return graph->GetFileData(file, left, right, mode);
}

void GraphManager::UpdateGraphData(const std::string &tag, const std::string &file, std::vector<ScalarPoint> &&data)
{
    if (!GraphExits(tag)) {
        auto graph = std::make_shared<Graph>(tag);
        graphs_.emplace(tag, graph);
    }
    std::shared_ptr<Graph> graph = GetGraph(tag);
    if (graph) {
        return graph->UpdateData(file, std::move(data));
    }
}

std::shared_ptr<Graph> GraphManager::GetGraph(const std::string &tag)
{
    if (!GraphExits(tag)) {
        return nullptr;
    }
    return graphs_.at(tag);
}

bool GraphManager::GraphExits(const std::string &tag)
{
    return graphs_.find(tag) != graphs_.end();
}

void GraphManager::Reset()
{
    graphs_.clear();
}

std::unordered_map<std::string, std::vector<std::string> > GraphManager::GetAllGraphInfo()
{
    std::unordered_map<std::string, std::vector<std::string> > res;
    for (const auto &[tag, graph]: graphs_) {
        res[tag] = graph->GetDataFiles();
    }
    return res;
}

void GraphManager::GetFileTags(std::string &path, std::set<std::string> &tags)
{
    for (auto &[tag, graph]: graphs_) {
        if (graph->InnerFile(path)) {
            tags.insert(tag);
        }
    }
}

void GraphManager::UpdateGraphSmoothingParam(const std::string &tag, const std::string &file,
                                             std::unique_ptr<SmoothingParamBase> param)
{
    if (!GraphExits(tag)) {
        return;
    }
    auto graph = GetGraph(tag);
    graph->UpdateSmoothingParam(file, std::move(param));
}
