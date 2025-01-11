#include <iostream>

#include <boost/asio/io_context.hpp>
#include <sdbusplus/asio/connection.hpp>

#include "routes.hpp"
#include "webapp.hpp"

constexpr const char* voltageProp = "Voltage";
constexpr const char* currentProp = "Current";
constexpr const char* windSpeedProp = "WindSpeed";
constexpr const char* windDirProp = "WindDir";
constexpr const char* IllimProp = "Illimunation";

int
main()
{
  // setup connection to dbus
  boost::asio::io_context io;
  auto conn = std::make_shared<sdbusplus::asio::connection>(io);

  crow::SimpleApp app;
  auto thread = std::thread([&io] { io.run(); });
  thread.detach();

  CROW_ROUTE(app, "/voltage")
  (
      [&io, &conn](crow::request& /*req*/, crow::response& res)
      {
        boost::asio::spawn(
            io,
            [conn, &res](boost::asio::yield_context yield)
            { webapp::getParam<double>(conn, yield, res, voltageProp); },
            boost::asio::detached);
      });
  CROW_ROUTE(app, "/current")
  (
      [&io, &conn](crow::request& /*req*/, crow::response& res)
      {
        boost::asio::spawn(
            io,
            [conn, &res](boost::asio::yield_context yield)
            { webapp::getParam<double>(conn, yield, res, currentProp); },
            boost::asio::detached);
      });
  CROW_ROUTE(app, "/windspeed")
  (
      [&io, &conn](crow::request& /*req*/, crow::response& res)
      {
        boost::asio::spawn(
            io,
            [conn, &res](boost::asio::yield_context yield)
            { webapp::getParam<double>(conn, yield, res, windSpeedProp); },
            boost::asio::detached);
      });
  CROW_ROUTE(app, "/winddir")
  (
      [&io, &conn](crow::request& /*req*/, crow::response& res)
      {
        boost::asio::spawn(
            io,
            [conn, &res](boost::asio::yield_context yield)
            { webapp::getParam<short>(conn, yield, res, windDirProp); },
            boost::asio::detached);
      });
  CROW_ROUTE(app, "/json")(
      []
      {
        crow::json::wvalue x({{"message", "Hello, World!"}});
        x["message2"] = "Hello, World.. Again!";
        return x;
      });

  app.port(18080).multithreaded().run();
}
