#pragma once

#include <ctime>
#include <string>
#include <variant>

#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/websocket.hpp>
#include <nlohmann/json.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/asio/property.hpp>
#include <sdbusplus/asio/sd_event.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/timer.hpp>

#include "crow.h"

namespace webapp
{
constexpr const char* ServiceName = "monitoring.test.service";
constexpr const char* ObjectPath = "/top/telemetry/watcher1";
constexpr const char* InterfaceName = "watcher1.interface.test";
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
    crow::response(400);
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
          res.end();
          return;
        }
        CROW_LOG_ERROR << "connection is already dead";
        crow::response(400);
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

}  // namespace webapp