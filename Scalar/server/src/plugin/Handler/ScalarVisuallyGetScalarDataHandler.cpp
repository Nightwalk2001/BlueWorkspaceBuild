/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#include "ScalarVisuallyGetScalarDataHandler.h"
#include <cmath>
#include "Logger.h"
#include "ScalarVisuallyServer.h"
#include "LineOp/LineOp.h"
#include "LineOp/LineOpFactory.h"

using namespace Insight::Scalar;
using namespace Insight::Scalar::Protocol;

bool ScalarVisuallyGetScalarDataHandler::run(std::string_view data, std::string& resultStr)
{
    resultStr = GetBasicResponse();
    std::string errMsg;
    GetScalarDataRequest request;
    if (auto errCode = ParseRequestFromJson(data, request, errMsg); errCode != ErrCode::OK) {
        LOG(LogRank::Error) << "Invalid request, error:" << errMsg;
        SetResponseError(errCode, errMsg, resultStr);
        return false;
    }

    std::vector<ScalarResponse> responseData;
    for (const auto& graphReq : request.data_) {
        responseData.emplace_back(ProcessSingleGraphReq(graphReq));
    }
    SetResponse(std::move(responseData), resultStr);
    return true;
}


bool ScalarVisuallyGetScalarDataHandler::CheckParamValid(const document_t& request)
{
    if (!request.HasMember("graphList") || !request["graphList"].IsArray()) {
        return false;
    }
    return std::all_of(request["graphList"].Begin(), request["graphList"].End(), [](const auto& item) {
        if (!item.HasMember("tag") || !item["tag"].IsString()) {
            return false;
        }
        if (!item.HasMember("file") || !item["file"].IsString()) {
            return false;
        }
        if (!item.HasMember("start") || !item["start"].IsInt64()) {
            return false;
        }
        if (!item.HasMember("end") || !item["end"].IsInt64()) {
            return false;
        }
        if (!item.HasMember("graphConfig") || !item["graphConfig"].IsArray()) {
            return false;
        }
        return true;
    });
}

void ScalarVisuallyGetScalarDataHandler::SetResponse(std::vector<ScalarResponse>&& responseData, std::string& resultStr)
{
    document_t document = ParseJsonToStr(resultStr);
    auto& allocator = document.GetAllocator();
    json data(rapidjson::kArrayType);
    for (auto& graphData : responseData) {
        json graph(rapidjson::kObjectType);
        AddJsonMember(graph, "tag", graphData.tag_, allocator);
        AddJsonMember(graph, "file", graphData.file_, allocator);
        for (auto& lineData : graphData.lines_) {
            json line(rapidjson::kObjectType);
            for (auto& point : lineData.data_) {
                json scalar(rapidjson::kObjectType);
                // process float Value Nan and Inf
                if (std::isnan(point.value_) || std::isinf(point.value_)) {
                    AddJsonMember(scalar, "value", std::to_string(point.value_), allocator);
                }
                else {
                    AddJsonMember(scalar, "value", point.value_, allocator);
                }
                AddJsonMember(scalar, "wallTime", point.wallTime_, allocator);
                AddJsonMember(scalar, "date", point.GetLocalTime(), allocator);
                AddJsonMember(line, std::to_string(point.step_), scalar, allocator);
            }
            AddJsonMember(graph, lineData.lineType_, line, allocator);
        }
        json dateConfig(rapidjson::kArrayType);
        for (auto& dateItem : graphData.dateIndex_) {
            json date(rapidjson::kObjectType);
            AddJsonMember(date, "step", std::get<0>(dateItem), allocator);
            AddJsonMember(date, "value", std::get<1>(dateItem), allocator);
            AddJsonMember(date, "date", std::get<2>(dateItem), allocator);
            dateConfig.PushBack(date, allocator);
        }
        AddJsonMember(graph, "dateConfig", dateConfig, allocator);
        data.PushBack(graph, allocator);
    }
    AddJsonMember(document["body"], "data", data, allocator);
    resultStr = DumpJsonToStr(document);
}

ErrCode ScalarVisuallyGetScalarDataHandler::ParseRequestFromJson(std::string_view data, GetScalarDataRequest& request,
                                                                 std::string& errMsg)
{
    std::string parseErr;
    std::optional<document_t> document = TryParseJson<rapidjson::ParseFlag::kParseDefaultFlags>(data, parseErr);
    if (!document.has_value()) {
        errMsg = "Invalid request json, err:" + parseErr;
        return ErrCode::INVALID_REQUEST_JSON;
    }
    if (!CheckParamValid(document.value())) {
        errMsg = "Invalid request param";
        return ErrCode::REQUEST_INVALID_PARAM;
    }
    const json& graphList = document.value()["graphList"];
    std::for_each(graphList.Begin(), graphList.End(), [&request, &document](const json& graph) {
        SingleGraphReqInfo temp;
        temp.tag_ = graph["tag"].GetString();
        temp.file_ = graph["file"].GetString();
        temp.start_ = graph["start"].GetInt64();
        temp.end_ = graph["end"].GetInt64();
        const json_t& graphConfigs = graph["graphConfig"];
        auto& allocator = document->GetAllocator();
        std::for_each(graphConfigs.Begin(), graphConfigs.End(), [&temp, & allocator](const json_t& conf) {
            document_t confJson;
            confJson.CopyFrom(conf, confJson.GetAllocator());
            temp.graphLines.emplace_back(std::move(confJson));
        });
        request.data_.emplace_back(std::move(temp));
    });
    return ErrCode::OK;
}

std::unordered_map<LineType, std::shared_ptr<LineOp>> ScalarVisuallyGetScalarDataHandler::BuildLineOP(
    const SingleGraphReqInfo& graphReq)
{
    std::unordered_map<LineType, std::shared_ptr<LineOp>> opMap;
    LineBasicInfo basicInfo = {graphReq.tag_, graphReq.file_, graphReq.start_, graphReq.end_};
    std::for_each(graphReq.graphLines.begin(), graphReq.graphLines.end(), [&opMap, &basicInfo](const json_t& param) {
        BuildLineOps(param, basicInfo, opMap);
    });
    // 默认添加sampleOp
    auto line = std::make_shared<SampleLineOp>();
    line->SetBasicInfo(graphReq.tag_, graphReq.file_, graphReq.start_, graphReq.end_);
    opMap.emplace(LineType::SAMPLE, line);
    if (opMap.find(LineType::NORMAL) == opMap.end()) {
        auto normal = std::make_shared<NormalLineOp>();
        normal->SetBasicInfo(graphReq.tag_, graphReq.file_, graphReq.start_, graphReq.end_);
        opMap.emplace(LineType::NORMAL, normal);
    }
    if (opMap.find(LineType::TOKEN) != opMap.end()) {
        opMap[LineType::NORMAL]->SetDataMode(DataMode::TOKEN_NORMAL);
        if (opMap.find(LineType::SMOOTHING) != opMap.end()) {
            opMap[LineType::SMOOTHING]->SetDataMode(DataMode::TOKEN_SMOOTHING);
        }
    }
    return opMap;
}


ScalarResponse ScalarVisuallyGetScalarDataHandler::ProcessSingleGraphReq(const SingleGraphReqInfo& graphReq)
{
    ScalarResponse graph;
    graph.file_ = graphReq.file_;
    graph.tag_ = graphReq.tag_;
    std::unordered_map<LineType, std::shared_ptr<LineOp>> opMap = BuildLineOP(graphReq);
    std::vector<GraphLine> lines = BuildLines(opMap);

    using LineOps = std::vector<std::shared_ptr<LineOp>>;
    std::unordered_map<LineType, LineOps> lineOpMap{};
    std::for_each(lines.begin(), lines.end(), [&lineOpMap, &opMap, this](const GraphLine& line) {
        lineOpMap[line.GetType()] = GetLineOps(line.GetType(), opMap);
    });

    for (auto& line : lines) {
        LineOps& ops = lineOpMap[line.GetType()];
        std::for_each(ops.begin(), ops.end(), [&line](std::shared_ptr<LineOp> op) {
            op->Process(line);
        });
        LineData lineData;
        lineData.lineType_ = CastLineTypeStr(line.GetType());
        lineData.data_ = line.GetLineData();
        if (graph.dateIndex_.empty() && !lineData.data_.empty()) {
            graph.dateIndex_ = BuildDateIndex(lineData.data_.begin(),
                                              lineData.data_.end());
        }
        graph.lines_.emplace_back(std::move(lineData));
    }
    return graph;
}


std::vector<GraphLine> ScalarVisuallyGetScalarDataHandler::BuildLines(
    const std::unordered_map<LineType, std::shared_ptr<LineOp>>& opMap)
{
    return GraphLine::BuildLine(opMap);
}

std::vector<std::shared_ptr<LineOp>> ScalarVisuallyGetScalarDataHandler::GetLineOps(
    LineType type, const std::unordered_map<LineType, std::shared_ptr<LineOp>>& opMap)
{
    std::vector<std::shared_ptr<LineOp>> res;
    switch (type) {
    case LineType::NORMAL:
        {
            res.emplace_back(opMap.at(LineType::NORMAL)->Clone());
            res.emplace_back(opMap.at(LineType::SAMPLE)->Clone());
            break;
        }
    case LineType::NORMAL_SMOOTHING:
    case LineType::SMOOTHING:
        {
            res.emplace_back(opMap.at(LineType::SMOOTHING)->Clone());
            res.emplace_back(opMap.at(LineType::SAMPLE)->Clone());
            break;
        }
    case LineType::TOKEN:
        {
            res.emplace_back(opMap.at(LineType::NORMAL)->Clone());
            res.emplace_back(opMap.at(LineType::SAMPLE)->Clone());
            res.emplace_back(opMap.at(LineType::TOKEN)->Clone());
            break;
        }
    case LineType::TOKEN_SMOOTHING:
        {
            res.emplace_back(opMap.at(LineType::SMOOTHING)->Clone());
            res.emplace_back(opMap.at(LineType::SAMPLE)->Clone());
            res.emplace_back(opMap.at(LineType::TOKEN)->Clone());
            break;
        }
    case LineType::SAMPLE:
    case LineType::UNKNOWN:
        break;
    }
    return res;
}


std::vector<std::tuple<uint64_t, double, std::string>>
ScalarVisuallyGetScalarDataHandler::BuildDateIndex(std::vector<ScalarPoint>::iterator begin,
                                                   std::vector<ScalarPoint>::iterator end)
{
    std::vector<std::tuple<uint64_t, double, std::string>> res;
    std::string lastDate;
    for (auto it = begin; it != end; it++) {
        auto pos = it->localTime_.find(',');
        std::string date = it->localTime_.substr(0, pos - 9);
        if (date.empty()) {
            lastDate = date;
            continue;
        }
        if (date != lastDate) {
            lastDate = date;
            res.emplace_back(it->step_, it->value_, date);
        }
    }
    return res;
}
