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
  using namespace webapp;
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
            [conn,
             res = std::move(res)](boost::asio::yield_context yield) mutable
            {
              boost::system::error_code ec;
              // get timestamp
              auto paramName = "Voltage";
              std::string date =
                  conn->yield_method_call<std::string>(yield,
                                                       ec,
                                                       ServiceName,
                                                       ObjectPath,
                                                       InterfaceName,
                                                       "getTimestamp");
              if (ec) {
                CROW_LOG_ERROR << "error getting timestamp";
                crow::response(400);
              }
              // get property
              sdbusplus::asio::getProperty<double>(
                  *conn,
                  ServiceName,
                  ObjectPath,
                  InterfaceName,
                  paramName,
                  [&res,
                   pName = std::move(paramName),
                   timestamp = std::move(date)](boost::system::error_code ecode,
                                                double value) mutable
                  {
                    if (ecode) {
                      CROW_LOG_ERROR << "error getting property: " << pName;
                      crow::response(400);
                    }
                    //if (res.is_alive()) {
                      crow::json::wvalue x;
                      x[pName] = value;
                      x["Timestamp"] = timestamp;
                      res.write(x.dump());
                      res.end();
                      return;
                    //}
                    //CROW_LOG_ERROR << "connection is already dead";
                    //crow::response(400);
                    //res.end();
                  });
            },
            boost::asio::detached);
      });
  CROW_ROUTE(app, "/json")(
      []
      {
        crow::json::wvalue x({{"message", "Hello, World!"}});
        x["message2"] = "Hello, World.. Again!";
        return x;
      });

  // app.port(18080).loglevel(crow::LogLevel::DEBUG).run();
  app.port(18080).multithreaded().run_async();
  // app.port(18080).loglevel(crow::LogLevel::DEBUG).multithreaded().run();
}
