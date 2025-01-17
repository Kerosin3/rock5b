#pragma once


enum class JsonParseResult
{
    BadContentType,
    BadJsonData,
    Success,
};


inline JsonParseResult parseRequestAsJson(const crow::request& req,
                                          nlohmann::json& jsonOut)
{
    jsonOut = nlohmann::json::parse(req.body, nullptr, false);
    if (jsonOut.is_discarded())
    {
        return JsonParseResult::BadJsonData;
    }

    return JsonParseResult::Success;
}