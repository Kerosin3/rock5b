#pragma once

#include <cstdint>
#include <ctime>
#include <map>
#include <string>
#include <variant>

#include <nlohmann/json.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/asio/property.hpp>
#include <sdbusplus/bus.hpp>

#include "crow.h"
#include "processPatch.hpp"

namespace webapp
{
// monitor bus
constexpr const char* ServiceName = "monitoring.test.service";
constexpr const char* ObjectPath = "/top/telemetry/watcher1";
constexpr const char* InterfaceName = "watcher1.interface.test";

// setup bus
constexpr const char* FreeDeskopPath = "org.freedesktop.DBus.Properties";
constexpr const char* ServiceSetup = "setup.test.service";
constexpr const char* ObjectSetup = "/top/setup/device";
constexpr const char* InterfaceSetup = "device.interface.test";

using variant = std::variant<int, std::string>;

template<typename T>
inline void
getParam(std::shared_ptr<sdbusplus::asio::connection> conn,
         boost::asio::yield_context yield,
         crow::response& res,
         const std::string& paramName)
{
  using json = nlohmann::json;
  boost::system::error_code ec;

  try {
    std::string timestamp = conn->yield_method_call<std::string>(
        yield, ec, ServiceName, ObjectPath, InterfaceName, "getTimestamp");

    if (ec && res.is_alive()) {
      CROW_LOG_ERROR << "Error getting timestamp";
      res.code = 500;
      res.end();
      return;
    }
    sdbusplus::asio::getProperty<T>(
        *conn,
        ServiceName,
        ObjectPath,
        InterfaceName,
        paramName,
        [&res,
         paramName = std::move(paramName),
         timestamp = std::move(timestamp)](boost::system::error_code ecode,
                                           T value) mutable
        {
          if (ecode && res.is_alive()) {
            CROW_LOG_ERROR << "Error getting property: " << paramName;
            res.code = 400;
            res.end();
            return;
          }

          if (res.is_alive()) {
            res.code = 200;
            json wrapper = json::object();
            wrapper[paramName] = value;
            wrapper["Time"] = timestamp;
            res.sendJSON(wrapper);
          } else if (res.is_alive()){
            res.code = 500;
            res.end();
          } else {
          }
        });
  } catch (...) {
    std::cout << "error!\n";
    res.code = 500;
    res.end();
  }
}

template<typename T>
inline void
getRetData(std::shared_ptr<sdbusplus::asio::connection> conn,
           boost::asio::yield_context yield,
           crow::response& res,
           const std::string& paramName)
{
  using json = nlohmann::json;
  boost::system::error_code ec;

  std::string timestamp = conn->yield_method_call<std::string>(
      yield, ec, ServiceName, ObjectPath, InterfaceName, "getTimestamp");

  if (ec) {
    CROW_LOG_ERROR << "Error getting timestamp";
    res.code = 500;
    res.end();
    return;
  }

  sdbusplus::asio::getProperty<T>(
      *conn,
      ServiceName,
      ObjectPath,
      InterfaceName,
      paramName,
      [&res,
       paramName = std::move(paramName),
       timestamp = std::move(timestamp)](boost::system::error_code ecode,
                                         T value) mutable
      {
        if (ecode) {
          CROW_LOG_ERROR << "Error getting property: " << paramName;
          // res.code = 400;
          // res.end();
          return;
        }

        if (res.is_alive()) {
          res.code = 200;
          res.set_header("Content-Type", "application/json");
          crow::json::wvalue x;
          x[paramName] = value;
          res.write(x.dump());
          res.end();
        } else {
          res.code = 500;
          res.end();
        }
      });
}

inline void
getData(std::shared_ptr<sdbusplus::asio::connection> conn,
        boost::asio::yield_context yield,
        crow::response& res)
{
  boost::system::error_code ec;

  std::string timestamp = conn->yield_method_call<std::string>(
      yield, ec, ServiceName, ObjectPath, InterfaceName, "getTimestamp");
  if (ec) {
    CROW_LOG_ERROR << "Error getting timestamp";
    res.code = 500;
    res.end();
    return;
  }

  sdbusplus::asio::getAllProperties(
      *conn,
      ServiceName,
      ObjectPath,
      InterfaceName,
      [&res, timestamp = std::move(timestamp)](
          const boost::system::error_code ecode,
          const std::vector<std::pair<std::string,
                                      std::variant<std::monostate,
                                                   std::string,
                                                   int64_t,
                                                   double,
                                                   int16_t>>>& properties)
          -> void
      {
        if (ecode) {
          CROW_LOG_ERROR << "Error getting properties";
          res.code = 400;
          res.end();
          return;
        }

        nlohmann::json::array_t messages;

        for (const auto& [key, val] : properties) {
          nlohmann::json::object_t obj;

          if (std::holds_alternative<std::string>(val)) {
            obj[key] = std::get<std::string>(val);
          } else if (std::holds_alternative<double>(val)) {
            obj[key] = std::get<double>(val);
          } else if (std::holds_alternative<int64_t>(val)) {
            obj[key] = std::get<int64_t>(val);
          } else if (std::holds_alternative<int16_t>(val)) {
            obj[key] = std::get<int16_t>(val);
          } else {
            continue;
          }

          messages.push_back(std::move(obj));
        }

        nlohmann::json outjson;
        outjson["CurrentData"] = std::move(messages);
        outjson["Time"] = timestamp;
        res.sendJSON(outjson);
        res.end();
      });
}

inline void
changeSettings(std::shared_ptr<sdbusplus::asio::connection> conn,
               boost::asio::yield_context yield,
               crow::response& res,
               crow::request& req)
{
  using json = nlohmann::json;
  using variant = std::variant<std::map<std::string, int32_t>, std::string>;

  std::optional<nlohmann::json::object_t> Data;

  // test Settings field
  if (!readJsonPatch(req, res, "Settings", Data)) {
    res.code = 500;
    res.end();
    return;
  }
  // process patch
  auto datax = *std::move(Data);
  std::map<std::string, int32_t> dataMap {};
  for (const auto& [settingname, settingvalue] : datax) {
    dataMap[settingname] = settingvalue;
  }

  // set data to dbus
  boost::system::error_code ec;
  conn->yield_method_call<>(yield,
                            ec,
                            ServiceSetup,
                            ObjectSetup,
                            FreeDeskopPath,
                            "Set",
                            InterfaceSetup,
                            "Settings",
                            variant(dataMap));

  if (ec) {
    res.code = 500;
  } else {
    res.code = 200;
    json wrapper = json::object();
    wrapper["Status"] = "Ok";
    res.sendJSON(wrapper);
  }
  res.end();
}

inline void
getSettings(std::shared_ptr<sdbusplus::asio::connection> conn,
            boost::asio::yield_context yield,
            crow::response& res)
{
  boost::system::error_code ec;
  using json = nlohmann::json;
  using variant = std::variant<std::map<std::string, int32_t>>;

  variant settingsData;

  settingsData = conn->yield_method_call<variant>(yield,
                                                  ec,
                                                  ServiceSetup,
                                                  ObjectSetup,
                                                  FreeDeskopPath,
                                                  "Get",
                                                  InterfaceSetup,
                                                  "Settings");

  if (ec) {
    res.code = 500;
    res.write("Failed to fetch settings: " + ec.message());
    res.end();
    return;
  }

  if (auto* settings =
          std::get_if<std::map<std::string, int32_t>>(&settingsData))
  {
    res.code = 200;

    auto time =
        std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    auto timestamp = std::format("{:%Y-%m-%d %X}", time);

    json wrapper = json::object();
    wrapper["Time"] = timestamp;

    json data = json::array();
    for (const auto& [key, value] : *settings) {
      data.push_back({key, value});
    }

    wrapper["Settings"] = std::move(data);
    res.sendJSON(wrapper);
  } else {
    res.code = 500;
    res.write("Unexpected data format received.");
    res.end();
  }
}

/*
inline void
test_interface2(std::shared_ptr<sdbusplus::asio::connection> conn,
                boost::asio::yield_context yield,
                crow::response& res,
                crow::request& req)
{
  sdbusplus::asio::getProperty<double>(
      *conn,
      ServiceName,
      ObjectPath,
      InterfaceName,
      "Voltage",
      [&res](boost::system::error_code ec, double value) mutable
      {
        if (ec) {
          std::cout << "errror!\n";
          return;
        }
        std::cout << "Greetings value is: " << value << "\n";
        if (res.is_alive()) {
          crow::json::wvalue x;
          x["test"] = value;
          res.write(x.dump());
          std::cout << "exit\n";
          // res.end();
          return;
        }
        // res.end();
      });
  //   response["test"] = std::get<int>(testValue);
}
*/

}  // namespace webapp
