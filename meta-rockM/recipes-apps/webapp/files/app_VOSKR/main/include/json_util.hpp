#pragma once

#include <crow.h>
#include "parsing.hpp"

namespace details
{

template<typename Type>
struct IsOptional : std::false_type
{
};

template<typename Type>
struct IsOptional<std::optional<Type>> : std::true_type
{
};

template<typename Type>
struct IsVector : std::false_type
{
};

template<typename Type>
struct IsVector<std::vector<Type>> : std::true_type
{
};

template<typename Type>
struct IsStdArray : std::false_type
{
};

template<typename Type, std::size_t size>
struct IsStdArray<std::array<Type, size>> : std::true_type
{
};

template<typename Type>
struct IsVariant : std::false_type
{
};

template<typename... Types>
struct IsVariant<std::variant<Types...>> : std::true_type
{
};

enum class UnpackErrorCode
{
  success,
  invalidType,
  outOfRange
};

template<typename ToType, typename FromType>
bool
checkRange(const FromType& from [[maybe_unused]],
           std::string_view key [[maybe_unused]])
{
  if constexpr (std::is_floating_point_v<ToType>) {
    if (std::isnan(from)) {
      return false;
    }
  }
  if constexpr (std::numeric_limits<ToType>::max()
                < std::numeric_limits<FromType>::max())
  {
    if (from > std::numeric_limits<ToType>::max()) {
      return false;
    }
  }
  if constexpr (std::numeric_limits<ToType>::lowest()
                > std::numeric_limits<FromType>::lowest())
  {
    if (from < std::numeric_limits<ToType>::lowest()) {
      return false;
    }
  }

  return true;
}

template<typename Type>
UnpackErrorCode
unpackValueWithErrorCode(nlohmann::json& jsonValue,
                         std::string_view key,
                         Type& value)
{
  UnpackErrorCode ret = UnpackErrorCode::success;

  if constexpr (std::is_floating_point_v<Type>) {
    double helper = 0;
    double* jsonPtr = jsonValue.get_ptr<double*>();

    if (jsonPtr == nullptr) {
      int64_t* intPtr = jsonValue.get_ptr<int64_t*>();
      if (intPtr != nullptr) {
        helper = static_cast<double>(*intPtr);
        jsonPtr = &helper;
      }
    }
    if (jsonPtr == nullptr) {
      return UnpackErrorCode::invalidType;
    }
    if (!checkRange<Type>(*jsonPtr, key)) {
      return UnpackErrorCode::outOfRange;
    }
    value = static_cast<Type>(*jsonPtr);
  }

  else if constexpr (std::is_signed_v<Type>)
  {
    int64_t* jsonPtr = jsonValue.get_ptr<int64_t*>();
    if (jsonPtr == nullptr) {
      return UnpackErrorCode::invalidType;
    }
    if (!checkRange<Type>(*jsonPtr, key)) {
      return UnpackErrorCode::outOfRange;
    }
    value = static_cast<Type>(*jsonPtr);
  }

  else if constexpr ((std::is_unsigned_v<Type>)
                     && (!std::is_same_v<bool, Type>))
  {
    uint64_t* jsonPtr = jsonValue.get_ptr<uint64_t*>();
    if (jsonPtr == nullptr) {
      return UnpackErrorCode::invalidType;
    }
    if (!checkRange<Type>(*jsonPtr, key)) {
      return UnpackErrorCode::outOfRange;
    }
    value = static_cast<Type>(*jsonPtr);
  }

  else if constexpr (std::is_same_v<nlohmann::json, Type>)
  {
    value = std::move(jsonValue);
  } else if constexpr (std::is_same_v<std::nullptr_t, Type>) {
    if (!jsonValue.is_null()) {
      return UnpackErrorCode::invalidType;
    }
  } else if constexpr (IsVector<Type>::value) {
    nlohmann::json::object_t* obj =
        jsonValue.get_ptr<nlohmann::json::object_t*>();
    if (obj == nullptr) {
      return UnpackErrorCode::invalidType;
    }

    for (const auto& val : *obj) {
      value.emplace_back();
      ret = unpackValueWithErrorCode<typename Type::value_type>(
                val, key, value.back())
          && ret;
    }
  } else {
    using JsonType = std::add_const_t<std::add_pointer_t<Type>>;
    JsonType jsonPtr = jsonValue.get_ptr<JsonType>();
    if (jsonPtr == nullptr) {
                             return UnpackErrorCode::invalidType;
    }
    value = std::move(*jsonPtr);
  }
  return ret;
}

template<std::size_t Index = 0, typename... Args>
UnpackErrorCode
unpackValueVariant(nlohmann::json& j,
                   std::string_view key,
                   std::variant<Args...>& v)
{
  if constexpr (Index < std::variant_size_v<std::variant<Args...>>) {
    std::variant_alternative_t<Index, std::variant<Args...>> type {};
    UnpackErrorCode unpack = unpackValueWithErrorCode(j, key, type);
    if (unpack == UnpackErrorCode::success) {
      v = std::move(type);
      return unpack;
    }

    return unpackValueVariant<Index + 1, Args...>(j, key, v);
  }
  return UnpackErrorCode::invalidType;
}


template<typename Type>
bool
unpackValue(nlohmann::json& jsonValue,
            std::string_view key,
            crow::response& res,
            Type& value)
{
  bool ret = true;

  if constexpr (IsOptional<Type>::value) {
    value.emplace();
    ret = unpackValue<typename Type::value_type>(jsonValue, key, res, *value)
        && ret;
  } else if constexpr (IsStdArray<Type>::value) {
    nlohmann::json::array_t* arr =
        jsonValue.get_ptr<nlohmann::json::array_t*>();
    if (arr == nullptr) {
      return false;
    }
    if (jsonValue.size() != value.size()) {
      return false;
    }
    size_t index = 0;
    for (auto& val : *arr) {
      ret =
          unpackValue<typename Type::value_type>(val, key, res, value[index++])
          && ret;
    }
  } else if constexpr (IsVector<Type>::value) {
    nlohmann::json::array_t* arr =
        jsonValue.get_ptr<nlohmann::json::array_t*>();
    if (arr == nullptr) {
      return false;
    }

    for (auto& val : *arr) {
      value.emplace_back();
      ret = unpackValue<typename Type::value_type>(val, key, res, value.back())
          && ret;
    }
  } else if constexpr (IsVariant<Type>::value) {
    UnpackErrorCode ec = unpackValueVariant(jsonValue, key, value);
    if (ec != UnpackErrorCode::success) {
      return false;
    }
  } else {
    UnpackErrorCode ec = unpackValueWithErrorCode(jsonValue, key, value);
    if (ec != UnpackErrorCode::success) {
      return false;
    }
  }

  return ret;
}

}  // namespace details

namespace json_util
{

inline bool
processJsonFromRequest(crow::response& res,
                       const crow::request& req,
                       nlohmann::json& reqJson)
{
  JsonParseResult ret = parseRequestAsJson(req, reqJson);
  if (ret == JsonParseResult::BadContentType) {
    return false;
  }
  reqJson = nlohmann::json::parse(req.body, nullptr, false);  // <====

  if (reqJson.is_discarded()) {
    return false;
  }

  return true;
}

// clang-format off
using UnpackVariant = std::variant<
    uint8_t*,
    uint16_t*,
    int16_t*,
    uint32_t*,
    int32_t*,
    uint64_t*,
    int64_t*,
    bool*,
    double*,
    std::string*,
    nlohmann::json::object_t*,
    std::variant<std::string, std::nullptr_t>*,
    std::variant<uint8_t, std::nullptr_t>*,
    std::variant<int16_t, std::nullptr_t>*,
    std::variant<uint16_t, std::nullptr_t>*,
    std::variant<int32_t, std::nullptr_t>*,
    std::variant<uint32_t, std::nullptr_t>*,
    std::variant<int64_t, std::nullptr_t>*,
    std::variant<uint64_t, std::nullptr_t>*,
    std::variant<double, std::nullptr_t>*,
    std::variant<bool, std::nullptr_t>*,
    std::vector<uint8_t>*,
    std::vector<uint16_t>*,
    std::vector<int16_t>*,
    std::vector<uint32_t>*,
    std::vector<int32_t>*,
    std::vector<uint64_t>*,
    std::vector<int64_t>*,
    //std::vector<bool>*,
    std::vector<double>*,
    std::vector<std::string>*,
    std::vector<nlohmann::json::object_t>*,
    std::optional<uint8_t>*,
    std::optional<uint16_t>*,
    std::optional<int16_t>*,
    std::optional<uint32_t>*,
    std::optional<int32_t>*,
    std::optional<uint64_t>*,
    std::optional<int64_t>*,
    std::optional<bool>*,
    std::optional<double>*,
    std::optional<std::string>*,
    std::optional<nlohmann::json::object_t>*,
    std::optional<std::vector<uint8_t>>*,
    std::optional<std::vector<uint16_t>>*,
    std::optional<std::vector<int16_t>>*,
    std::optional<std::vector<uint32_t>>*,
    std::optional<std::vector<int32_t>>*,
    std::optional<std::vector<uint64_t>>*,
    std::optional<std::vector<int64_t>>*,
    //std::optional<std::vector<bool>>*,
    std::optional<std::vector<double>>*,
    std::optional<std::vector<std::string>>*,
    std::optional<std::vector<nlohmann::json::object_t>>*,
    std::optional<std::variant<std::string, std::nullptr_t>>*,
    std::optional<std::variant<uint8_t, std::nullptr_t>>*,
    std::optional<std::variant<int16_t, std::nullptr_t>>*,
    std::optional<std::variant<uint16_t, std::nullptr_t>>*,
    std::optional<std::variant<int32_t, std::nullptr_t>>*,
    std::optional<std::variant<uint32_t, std::nullptr_t>>*,
    std::optional<std::variant<int64_t, std::nullptr_t>>*,
    std::optional<std::variant<uint64_t, std::nullptr_t>>*,
    std::optional<std::variant<double, std::nullptr_t>>*,
    std::optional<std::variant<bool, std::nullptr_t>>*,
    std::optional<std::vector<std::variant<nlohmann::json::object_t, std::nullptr_t>>>*,
    std::optional<std::vector<std::variant<std::string, nlohmann::json::object_t, std::nullptr_t>>>*,

    // Note, these types are kept for historical completeness, but should not be used,
    // As they do not provide object type safety.  Instead, rely on nlohmann::json::object_t
    // Will be removed Q2 2025
    nlohmann::json*,
    std::optional<std::vector<nlohmann::json>>*,
    std::vector<nlohmann::json>*,
    std::optional<nlohmann::json>*
>;
// clang-format on

struct PerUnpack
{
  std::string_view key;
  UnpackVariant value;
  bool complete = false;
};

inline void
packVariant(std::span<PerUnpack> /*toPack*/)
{
}

template<typename FirstType, typename... UnpackTypes>
void
packVariant(std::span<PerUnpack> toPack,
            std::string_view key,
            FirstType&& first,
            UnpackTypes&&... in)
{
  if (toPack.empty()) {
    return;
  }
  toPack[0].key = key;
  toPack[0].value = &first;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  packVariant(toPack.subspan(1), std::forward<UnpackTypes&&>(in)...);
}

inline bool readJsonHelper(nlohmann::json& jsonRequest, crow::response& res,
                           std::span<PerUnpack> toUnpack);

inline bool
readJsonHelperObject(nlohmann::json::object_t& obj,
                     crow::response& res,
                     std::span<PerUnpack> toUnpack)
{
  bool result = true;
  for (auto& item : obj) {
    size_t unpackIndex = 0;
    for (; unpackIndex < toUnpack.size(); unpackIndex++) {
      PerUnpack& unpackSpec = toUnpack[unpackIndex];
      std::string_view key = unpackSpec.key;
      size_t keysplitIndex = key.find('/');
      std::string_view leftover;
      if (keysplitIndex != std::string_view::npos) {
        leftover = key.substr(keysplitIndex + 1);
        key = key.substr(0, keysplitIndex);
      }

      if (key != item.first || unpackSpec.complete) {
        continue;
      }

      // Sublevel key
      if (!leftover.empty()) {
        // Include the slash in the key so we can compare later
        key = unpackSpec.key.substr(0, keysplitIndex + 1);
        nlohmann::json j;
        result = details::unpackValue<nlohmann::json>(item.second, key, res, j)
            && result;
        if (!result) {
          return result;
        }

        std::vector<PerUnpack> nextLevel;
        for (PerUnpack& p : toUnpack) {
          if (!p.key.starts_with(key)) {
            continue;
          }
          std::string_view thisLeftover = p.key.substr(key.size());
          nextLevel.push_back({thisLeftover, p.value, false});
          p.complete = true;
        }

        result = readJsonHelper(j, res, nextLevel) && result;
        break;
      }

      result = std::visit(
                   [&item, &unpackSpec, &res](auto& val)
                   {
                     using ContainedT =
                         std::remove_pointer_t<std::decay_t<decltype(val)>>;
                     return details::unpackValue<ContainedT>(
                         item.second, unpackSpec.key, res, *val);
                   },
                   unpackSpec.value)
          && result;

      unpackSpec.complete = true;
      break;
    }

    if (unpackIndex == toUnpack.size()) {
      result = false;
    }
  }

  for (PerUnpack& perUnpack : toUnpack) {
    if (!perUnpack.complete) {
      bool isOptional = std::visit(
          [](auto& val)
          {
            using ContainedType =
                std::remove_pointer_t<std::decay_t<decltype(val)>>;
            return details::IsOptional<ContainedType>::value;
          },
          perUnpack.value);
      if (isOptional) {
        continue;
      }
      result = false;
    }
  }
  return result;
}


inline bool readJsonHelper(nlohmann::json& jsonRequest, crow::response& res,
                           std::span<PerUnpack> toUnpack)
{
    nlohmann::json::object_t* obj =
        jsonRequest.get_ptr<nlohmann::json::object_t*>();
    if (obj == nullptr)
    {
        return false;
    }
    return readJsonHelperObject(*obj, res, toUnpack);
}

template<typename FirstType, typename... UnpackTypes>
bool
readJsonObject(nlohmann::json::object_t& jsonRequest,
               crow::response& res,
               std::string_view key,
               FirstType&& first,
               UnpackTypes&&... in)
{
  const std::size_t n = sizeof...(UnpackTypes) + 2;
  std::array<PerUnpack, n / 2> toUnpack2;
  packVariant(toUnpack2,
              key,
              std::forward<FirstType>(first),
              std::forward<UnpackTypes&&>(in)...);
  // return readJsonHelperObject(jsonRequest, res, toUnpack2);
  // return true;
  return readJsonHelperObject(jsonRequest, res, toUnpack2);
}

}  // namespace json_util