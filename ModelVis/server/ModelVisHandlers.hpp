#pragma once

#include "ModelVisPlugin.h"
#include "rust_ffi.h"

class LayoutHandler final : public PostHandler {
public:
    bool run(const std::string_view data, std::string &resultStr) override {
        resultStr = layout_rs(data.data());
        return true;
    };
};

class MiningSubGraphHandler final : public GetHandler {
    bool run(std::string_view data, std::string &result) override {
        return true;
    };
};
