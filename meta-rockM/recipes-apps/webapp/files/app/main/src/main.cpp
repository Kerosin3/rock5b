#include <cstdlib>
#include <iostream>
#include <variant>

#include <boost/asio/io_context.hpp>
#include <sdbusplus/asio/connection.hpp>

#include "processPatch.hpp"
#include "routes.hpp"
#include "webapp.hpp"

constexpr const char* voltageProp = "Voltage";
constexpr const char* currentProp = "Current";
constexpr const char* windSpeedProp = "WindSpeed";
constexpr const char* windDirProp = "WindDir";
constexpr const char* IllimProp = "Illimunation";

/*
void
handler(crow::request const& req, crow::response& res)
{
  std::optional<nlohmann::json::object_t> Data;
  if (!readJsonPatch(req, res, "Attributes", Data)) {
    return;
  }
  auto datax = *std::move(Data);
  for (const auto& [settingname, settingvalue] : datax) {
    std::cout << "vlue is" << settingvalue << "\n";
  }
  crow::json::wvalue x;
  x["done"] = 1;
  res.write(x.dump());
  res.end();
}
*/
int
main()
{
  try {
    // setup connection to dbus
    boost::asio::io_context io;
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);
    auto conn = std::make_shared<sdbusplus::asio::connection>(io);
    signals.async_wait([&io](const boost::system::error_code&, const int&)
                       { io.stop(); });
    // setup Crow
    crow::SimpleApp app;
    // run IO in detached thread
    auto thread = std::jthread([&] { io.run(); });
    //  establish routes
    CROW_ROUTE(app, "/voltage")
        .methods("GET"_method)(
            [&io, &conn](crow::request& req, crow::response& res)
            {
              req.close_connection = true;
              boost::asio::spawn(
                  io,
                  [conn, &res](boost::asio::yield_context yield)
                  { webapp::getParam<double>(conn, yield, res, voltageProp); },
                  boost::asio::detached);
            });
    CROW_ROUTE(app, "/current")
        .methods("GET"_method)(
            [&io, &conn](crow::request& /*req*/, crow::response& res)
            {
              boost::asio::spawn(
                  io,
                  [conn, &res](boost::asio::yield_context yield)
                  { webapp::getParam<double>(conn, yield, res, currentProp); },
                  boost::asio::detached);
            });
    CROW_ROUTE(app, "/windspeed")
        .methods("GET"_method)(
            [&io, &conn](crow::request& /*req*/, crow::response& res)
            {
              boost::asio::spawn(
                  io,
                  [conn, &res](boost::asio::yield_context yield)
                  {
                    webapp::getParam<double>(conn, yield, res, windSpeedProp);
                  },
                  boost::asio::detached);
            });
    CROW_ROUTE(app, "/winddir")
        .methods("GET"_method)(
            [&io, &conn](crow::request& /*req*/, crow::response& res)
            {
              boost::asio::spawn(
                  io,
                  [conn, &res](boost::asio::yield_context yield)
                  { webapp::getParam<short>(conn, yield, res, windDirProp); },
                  boost::asio::detached);
            });
    CROW_ROUTE(app, "/settings")
        .methods("GET"_method, "PATCH"_method)(
            [&io, &conn](crow::request& req, crow::response& res)
            {
              if (req.method == "GET"_method) {
                boost::asio::spawn(
                    io,
                    [conn, &req, &res](boost::asio::yield_context yield)
                    { webapp::getSettings(conn, yield, res); },
                    boost::asio::detached);

              } else {
                boost::asio::spawn(
                    io,
                    [conn, &req, &res](boost::asio::yield_context yield)
                    { webapp::changeSettings(conn, yield, res, req); },
                    boost::asio::detached);
              }
            });
    auto async_application = app.port(18080).multithreaded().run_async();
  } catch (...) {
    std::cerr << "Error: while running the programm " << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
