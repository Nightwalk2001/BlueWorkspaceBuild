/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#ifndef BOARD_PLUGINS_SCALARVISUALLY_SRC_PROTOCOLUTIL_PROTOCOLUTIL_H_
#define BOARD_PLUGINS_SCALARVISUALLY_SRC_PROTOCOLUTIL_PROTOCOLUTIL_H_

#include "defs/ConceptDefs.h"
#include "document.h"
#include "writer.h"

#ifdef _WIN32
#include <filesystem>
namespace fs = std::filesystem;
#else

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
#endif

#include <string>
#include <vector>
#include <string_view>
#include <set>
#include <optional>
#include "GraphLine.h"

const std::string kPathSeparator = "/";
namespace Insight::Scalar::Protocol {

struct SingleGraphReqInfo {
    std::string tag_;
    std::string file_;
    uint64_t start_;
    uint64_t end_;
    std::vector<document_t> graphLines;
};

struct GetScalarDataRequest {
    std::vector<SingleGraphReqInfo> data_;
};

using json = rapidjson::Value;
using document_t = rapidjson::Document;

template<unsigned parseFlags>
static inline std::optional<document_t> TryParseJson(std::string_view jsonStr, std::string &error)
{
    document_t doc;
    doc.Parse<parseFlags>(jsonStr.data(), jsonStr.length());
    if (doc.HasParseError()) {
        constexpr size_t printErrorSize = 10;
        auto offset = doc.GetErrorOffset();
        auto start = offset >= printErrorSize ? offset - printErrorSize : 0;
        error = "Error code:" + std::to_string(doc.GetParseError()) + ", str:" +
                std::string(jsonStr.substr(start, offset - start + printErrorSize));
        return std::nullopt;
    }
    return std::move(doc);
}

static inline document_t ParseJsonToStr(std::string_view jsonStr)
{
    document_t doc;
    doc.Parse<rapidjson::kParseDefaultFlags>(jsonStr.data(), jsonStr.length());
    return doc;
}

static inline std::string DumpJsonToStr(const json &document)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    return {buffer.GetString(), buffer.GetSize()};
}


template<typename T>
static inline void
AddJsonMember(json &dst, std::string_view key, T &&value, rapidjson::MemoryPoolAllocator<> &allocator)
{
    dst.AddMember(json().SetString(key.data(), allocator), std::forward<T>(value), allocator);
}

template<>
inline void AddJsonMember(json &dst, std::string_view key, std::vector<std::string> &&value,
                          rapidjson::MemoryPoolAllocator<> &allocator)
{
    json array(rapidjson::kArrayType);
    for (auto &item: value) {
        array.PushBack(json().SetString(item.c_str(), allocator), allocator);
    }
    dst.AddMember(rapidjson::StringRef(key.data(), key.length()), array, allocator);
}

static inline void
AddJsonMember(json &dst, std::string_view key, const std::string &value, rapidjson::MemoryPoolAllocator<> &allocator)
{
    dst.AddMember(rapidjson::StringRef(key.data(), key.length()),
                  rapidjson::Value().SetString(value.data(), value.size(), allocator), allocator);
}

static inline void
AddJsonMember(json &dst, std::string_view key, std::string &value, rapidjson::MemoryPoolAllocator<> &allocator)
{
    dst.AddMember(rapidjson::StringRef(key.data(), key.length()),
                  rapidjson::Value().SetString(value.data(), value.size(), allocator), allocator);
}

static inline void
AddJsonMember(json &dst, std::string_view key, std::string &&value, rapidjson::MemoryPoolAllocator<> &allocator)
{
    dst.AddMember(rapidjson::StringRef(key.data(), key.length()),
                  rapidjson::Value().SetString(value.data(), value.size(), allocator), allocator);
}

template<typename T>
static inline void
AddJsonMember(json &dst, std::string_view key, std::vector<T> &value, rapidjson::MemoryPoolAllocator<> &allocator)
{
    json temp(rapidjson::kArrayType);
    for (const T &item: value) {
        if constexpr (std::is_same_v<T, std::string>) {
            temp.PushBack(json().SetString(item.data(), item.size(), allocator), allocator);
        } else {
            temp.PushBack(std::forward<T>(item), allocator);
        }
    }
    AddJsonMember(dst, key, temp, allocator);
}

template<typename T>
static inline void
AddJsonMember(json &dst, std::string_view key, std::set<T> &value, rapidjson::MemoryPoolAllocator<> &allocator)
{
    json temp(rapidjson::kArrayType);
    for (const T &item: value) {
        if constexpr (std::is_same_v<T, std::string>) {
            temp.PushBack(json().SetString(item.data(), item.size(), allocator), allocator);
        } else {
            temp.PushBack(std::forward<T>(item), allocator);
        }
    }
    AddJsonMember(dst, key, temp, allocator);
}

static inline void SetResponseError(int errCode, const std::string &errMsg, std::string &resultStr)
{
    document_t result = ParseJsonToStr(resultStr);
    auto &allocator = result.GetAllocator();
    result["errCode"].SetInt(errCode);
    result["msg"].SetString(errMsg.c_str(), errMsg.size(), allocator);
    result["result"].SetBool(false);
    resultStr = DumpJsonToStr(result);
}

static inline std::string GetBasicResponse()
{
    return R"({"body":{}, "msg":"", "errCode":0, "result":true})";
}

static inline std::string GetReadableFileName(std::string_view path)
{
    if (path.empty()) {
        return "";
    }
    return fs::path(path).filename().string();
}
}
#endif //BOARD_PLUGINS_SCALARVISUALLY_SRC_PROTOCOLUTIL_PROTOCOLUTIL_H_
