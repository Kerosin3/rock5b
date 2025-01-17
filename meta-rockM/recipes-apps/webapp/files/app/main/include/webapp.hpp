#pragma once

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
  boost::system::error_code ec;
  // get timestamp
  std::string date = conn->yield_method_call<std::string>(
      yield, ec, ServiceName, ObjectPath, InterfaceName, "getTimestamp");
  if (ec) {
    CROW_LOG_ERROR << "error getting timestamp";
    res.code = 500;
    return;
  }
  // get property
  sdbusplus::asio::getProperty<T>(
      *conn,
      ServiceName,
      ObjectPath,
      InterfaceName,
      paramName,
      [&res, pName = std::move(paramName), timestamp = std::move(date)](
          boost::system::error_code ecode, T value) mutable
      {
        if (ecode) {
          CROW_LOG_ERROR << "error getting property: " << pName;
          crow::response(400);
        }
        if (res.is_alive()) {
          crow::json::wvalue x;
          x[pName] = value;
          x["Timestamp"] = timestamp;
          res.write(x.dump());
          res.code = 200;
          res.end();
          return;
        }
        res.code = 500;
        res.end();
      });
}

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

inline void
changeSettings(std::shared_ptr<sdbusplus::asio::connection> conn,
               boost::asio::yield_context yield,
               crow::response& res,
               crow::request& req)
{
  std::optional<nlohmann::json::object_t> Data;
  if (!readJsonPatch(req, res, "Settings", Data)) {
    res.code = 200;
    res.end();
    return;
  }
  auto datax = *std::move(Data);
  std::map<std::string, int32_t> dataMap {};
  for (const auto& [settingname, settingvalue] : datax) {
    dataMap[settingname] = settingvalue;
  }
  using variant = std::variant<std::map<std::string, int32_t>, std::string>;

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
    res.body = "settings have been changed!";
  }
  res.end();
}

inline void
getSettings(std::shared_ptr<sdbusplus::asio::connection> conn,
            boost::asio::yield_context yield,
            crow::response& res)
{
  boost::system::error_code ec;
  using variant = std::variant<std::map<std::string, int32_t>, std::string>;
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
  } else if (auto* settings =
                 std::get_if<std::map<std::string, int32_t>>(&settingsData))
  {
    res.code = 200;
    crow::json::wvalue x;
    for (auto& [k, v] : *settings) {
      x[k] = v;
    }
    res.write(x.dump());
  }
  res.end();
}

}  // namespace webapp
