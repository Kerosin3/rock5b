#pragma once

#include <boost/asio/io_context.hpp>
#include <nlohmann/json.hpp>
#include <sdbusplus/asio/connection.hpp>

#include "crow.h"
#include "webapp.hpp"

constexpr const char* voltageProp = "Voltage";
constexpr const char* currentProp = "Current";
constexpr const char* windSpeedProp = "WindSpeed";
constexpr const char* windDirProp = "WindDir";
constexpr const char* IllimProp = "Illimunation";

namespace handlers
{
void
handleVoltageRequest(std::shared_ptr<sdbusplus::asio::connection> conn,
                     boost::asio::io_context& io,
                     crow::response& res)
{
  boost::asio::spawn(io,
      [conn, &res](boost::asio::yield_context yield)
      { webapp::getParam<double>(conn, yield, res, voltageProp); },
	   boost::asio::detached);
}

void
handleDataRequest(std::shared_ptr<sdbusplus::asio::connection> conn,
                  boost::asio::io_context& io,
                  crow::response& res)
{
  boost::asio::spawn(io,
                     [conn, &res](boost::asio::yield_context yield)
                     { webapp::getData(conn, yield, res); });
}

void
handleCurrentRequest(std::shared_ptr<sdbusplus::asio::connection> conn,
                     boost::asio::io_context& io,
                     crow::response& res)
{
  boost::asio::spawn(
      io,
      [conn, &res](boost::asio::yield_context yield)
      { webapp::getParam<double>(conn, yield, res, currentProp); });
}

void
handleWindSpeedRequest(std::shared_ptr<sdbusplus::asio::connection> conn,
                       boost::asio::io_context& io,
                       crow::response& res)
{
  boost::asio::spawn(
      io,
      [conn, &res](boost::asio::yield_context yield)
      { webapp::getParam<double>(conn, yield, res, windSpeedProp); });
}

void
handleWindDirRequest(std::shared_ptr<sdbusplus::asio::connection> conn,
                     boost::asio::io_context& io,
                     crow::response& res)
{
  boost::asio::spawn(
      io,
      [conn, &res](boost::asio::yield_context yield)
      { webapp::getParam<short>(conn, yield, res, windDirProp); });
}

void
handleSettingsRequest(std::shared_ptr<sdbusplus::asio::connection> conn,
                      boost::asio::io_context& io,
                      crow::request& req,
                      crow::response& res)
{
  if (req.method == "GET"_method) {
    boost::asio::spawn(io,
                       [conn, &res](boost::asio::yield_context yield)
                       { webapp::getSettings(conn, yield, res); });
  } else {
    boost::asio::spawn(io,
                       [conn, &req, &res](boost::asio::yield_context yield)
                       { webapp::changeSettings(conn, yield, res, req); });
  }
}

void
handleTestRequest(crow::response& res)
{
  nlohmann::json xjson;
  nlohmann::json::array_t messages;
  nlohmann::json::object_t messagesobj;
  messages.emplace_back(std::move(messagesobj));
  xjson["Messages"] = std::move(messages);
  res.sendJSON(xjson);
}

}  // namespace handlers
