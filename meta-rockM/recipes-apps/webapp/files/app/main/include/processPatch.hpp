#pragma once

#include <optional>

#include <nlohmann/json.hpp>
#include "json_util.hpp"
#include <crow.h>

bool
processJsonFromRequest(crow::response& res,
                       const crow::request& req,
                       nlohmann::json& reqJson);

inline std::optional<nlohmann::json::object_t>
readJsonPatchHelper(const crow::request& req, crow::response& res)
{
  nlohmann::json jsonRequest;
  if (!json_util::processJsonFromRequest(res, req, jsonRequest)) {
    return std::nullopt;
  }
  nlohmann::json::object_t* object =
      jsonRequest.get_ptr<nlohmann::json::object_t*>();
  if (object == nullptr || object->empty()) {
    return std::nullopt;
  }
  std::erase_if(*object,
                [](const std::pair<std::string, nlohmann::json>& item)
                { return item.first.starts_with("@odata."); });
  if (object->empty()) {
    return std::nullopt;
  }

  return {std::move(*object)};
}

template<typename... UnpackTypes>
bool
readJsonPatch(const crow::request& req,
              crow::response& res,
              std::string_view key,
              UnpackTypes&&... in)
{
  std::optional<nlohmann::json::object_t> jsonRequest =
      readJsonPatchHelper(req, res);
  if (!jsonRequest) {
    return false;
  }
  if (jsonRequest->empty()) {
    return false;
  }

  return json_util::readJsonObject(
      *jsonRequest, res, key, std::forward<UnpackTypes&&>(in)...);
}