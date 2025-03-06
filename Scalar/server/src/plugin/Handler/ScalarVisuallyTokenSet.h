/*
* Copyright (c), Huawei Technologies Co., Ltd. 2025-2025.All rights reserved.
*/
#ifndef SCALARVISUALLYTOKENSET_H
#define SCALARVISUALLYTOKENSET_H
#include <set>
#include <thread>
#include <defs/ConceptDefs.h>
#include "ApiHandler.h"


using namespace Insight::Scalar;
namespace  Dic::Core {

struct TokenSetRequest
{
    std::vector<std::tuple<std::string, double, double>> tokenInfo_;
};


struct TokenSetResponseItem
{
    std::string file_;
    double globalBatchSize_;
    double seqLength_;
    std::set<std::string> tags_;
};

struct TokenSetResponse
{
    std::vector<TokenSetResponseItem> items_;
    std::string errMsg_;
};
class ScalarVisuallyTokenSet: public PostHandler {
public:
    ScalarVisuallyTokenSet() = default;
    ~ScalarVisuallyTokenSet() override = default;

    bool run(std::string_view data, std::string& result) override;

    static bool CheckParamValid(const Insight::Scalar::document_t& request);
private:
    static void SetResponse(TokenSetResponse&& response, std::string& resultStr);

    static ErrCode ParseReqeustFromJson(std::string_view data, TokenSetRequest &request, std::string &errMsg);

    /**
     * @brief 设置文件的token参数
     * @param file
     * @param globalBatchSize
     * @param seqLength
     * @param errMsg
     * @return 文件包含的tag,便于前端更新数据
     */
    std::set<std::string> SetFileToken(const std::string& file, double globalBatchSize, double seqLength, std::string& errMsg);
};
}





#endif //SCALARVISUALLYTOKENSET_H
