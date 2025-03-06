/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
*/
#ifndef BOARD_PLUGINS_SCALARVISUALLY_SRC_PROTOCOLUTIL_FILEUTIL_H_
#define BOARD_PLUGINS_SCALARVISUALLY_SRC_PROTOCOLUTIL_FILEUTIL_H_

#include <string>
#include <cstdint>
#include <map>
#include <vector>
#include <functional>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#include <stringapiset.h>
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

namespace Insight::Scalar {
class StrUtil {
public:
    static inline uint8_t ByteNum(uint8_t byte) {
        const static std::map<uint8_t, uint8_t> MAP_BYTES = {
                {0xE0, 0xC0},
                {0xF0, 0xE0},
                {0xF8, 0xF0},
                {0xFC, 0xF8},
                {0xFE, 0xFC}
        };
        uint8_t index = 0;
        uint8_t byteNum = 0;
        for (auto [k, v]: MAP_BYTES) {
            index++;
            if ((byte & k) == v) {
                byteNum = index;
                break;
            }
        }
        return byteNum;
    }

    static inline bool IsUtf8String(const std::string &str) {
        uint32_t byteNum = 0;
        for (const char c: str) {
            const auto byte = static_cast<uint8_t>(c);
            if ((byteNum == 0) && ((byte & 0x80) == 0)) {
                continue;
            }
            if (byteNum == 0) {
                byteNum = ByteNum(byte);
                if (byteNum == 0) {
                    return false;
                }
            } else {
                if ((byte & 0xC0) != 0x80) {
                    return false;
                }
                byteNum--;
            }
        }
        return true;
    }

#ifdef _WIN32
    static inline std::string Utf8ToGbk(const char *src)
    {
        if (src == nullptr) {
            return "";
        }
        int len = MultiByteToWideChar(CP_UTF8, 0, src, -1, nullptr, 0);
        const auto wstr = std::make_unique<wchar_t[]>(len + 1);
        MultiByteToWideChar(CP_UTF8, 0, src, -1, wstr.get(), len);
        len = WideCharToMultiByte(CP_ACP, 0, wstr.get(), -1, nullptr, 0, nullptr, nullptr);
        const auto str = std::make_unique<char[]>(len + 1);
        WideCharToMultiByte(CP_ACP, 0, wstr.get(), -1, str.get(), len, nullptr, nullptr);
        return {str.get()};
    }

    static inline std::string GbkToUtf8(const char *src)
    {
        if (src == nullptr) {
            return "";
        }
        int len = MultiByteToWideChar(CP_ACP, 0, src, -1, nullptr, 0);
        const auto wstr = std::make_unique<wchar_t[]>(len + 1);
        MultiByteToWideChar(CP_ACP, 0, src, -1, wstr.get(), len);
        len = WideCharToMultiByte(CP_UTF8, 0, wstr.get(), -1, nullptr, 0, nullptr, nullptr);
        const auto str = std::make_unique<char[]>(len + 1);
        WideCharToMultiByte(CP_UTF8, 0, wstr.get(), -1, str.get(), len, nullptr, nullptr);
        return {str.get()};
    }
#endif
};

class FileUtil {
public:
    static inline std::string PathPreProcess(std::string path) {
#ifdef WIN32
        if (StrUtil::IsUtf8String(path)) {
            path = StrUtil::Utf8ToGbk(path.c_str());
        }
#endif
        return path;
    }

    static inline bool FindFolder(const std::string_view path,
                                  std::vector<std::string> &folders,
                                  std::vector<std::string> &files) {
        if (path.empty()) {
            return false;
        }
        if (!fs::exists(path) || !fs::is_directory(path)) {
            return false;
        }
        for (auto &entry: fs::directory_iterator(path)) {
            if (std::string name = entry.path().filename().string(); name == "." || name == "..") {
                continue;
            }
            if (fs::is_directory(entry)) {
                folders.emplace_back(entry.path().string());
            } else if (fs::is_regular_file(entry)) {
                files.emplace_back(entry.path().string());
            }
        }
        return true;
    }

    static inline void ScanFolderIf(std::string_view path,
                                    std::vector<std::string> &dst,
                                    const std::function<bool(const fs::directory_entry &)> &func) {
        if (path.empty() || !fs::exists(path)) {
            return;
        }
        if (!fs::is_directory(path)) {
            return;
        }
        for (const auto &entry: fs::directory_iterator(path)) {
            if (func(entry)) {
                dst.emplace_back(path);
            }
        }
    }

    inline static std::vector<std::string> GetFileHierarchy(const std::string &pathPrefix, const std::string &path)
    {
        std::vector<std::string> result;
        if (path.find(pathPrefix) != 0) {
            return result;
        }
        std::string relativePath = path.substr(pathPrefix.size() + 1);
        // 将相对路径按/分割
        for (const auto &part: fs::path(relativePath)) {
            result.push_back(part.string());
        }
        return result;
    }
};


}
#endif //BOARD_PLUGINS_SCALARVISUALLY_SRC_PROTOCOLUTIL_FILEUTIL_H_
