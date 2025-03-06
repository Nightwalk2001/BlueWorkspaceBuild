/*
* Copyright (c), Huawei Technologies Co., Ltd. 2024-2024.All rights reserved.
 */
#ifndef TFEVENTPARSER_H
#define TFEVENTPARSER_H

#include "FileParser.h"
#include "proto/event.pb.h"
#include "defs/ConceptDefs.h"

namespace Insight::Scalar::Parser {
class TFEventParser final : public FileParser {
public:
    TFEventParser() {
        type_ = ParseDataType::TF_EVENT;
    }

    ~TFEventParser() override = default;
    bool ReadRecord(std::string &eventStr, std::ifstream &input) override;
    bool ParseRecordToScalar(std::string &&record, std::map<std::string, std::vector<ScalarPoint>> &res) override;

private:
    /**
     * @brief check whether contains scalar value
     * @param event tf event object
     * @return true for success
     */
    static bool EventContainsScalar(const tensorboard::Event &event);
};
}

#endif //TFEVENTPARSER_H
