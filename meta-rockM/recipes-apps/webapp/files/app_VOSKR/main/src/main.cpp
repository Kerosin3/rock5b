#include <cstdlib>
#include <iostream>
#include <thread>
#include <variant>

#include <boost/asio/io_context.hpp>
#include <nlohmann/json.hpp>
#include <sdbusplus/asio/connection.hpp>

#include "handlers.hpp"
#include "processPatch.hpp"
#include "routes.hpp"
#include "webapp.hpp"

std::vector<std::jthread> workers;
// monitor bus
constexpr const char* ServiceName = "monitoring.test.service";
constexpr const char* ObjectPath = "/top/telemetry/watcher1";
constexpr const char* InterfaceName = "watcher1.interface.test";

// setup bus
constexpr const char* FreeDeskopPath = "org.freedesktop.DBus.Properties";

class AsyncResp
{
public:
  AsyncResp() = default;
  explicit AsyncResp(crow::response&& resIn)
      : res(std::move(resIn))
  {
  }

  AsyncResp(const AsyncResp&) = delete;
  AsyncResp(AsyncResp&&) = delete;
  AsyncResp& operator=(const AsyncResp&) = delete;
  AsyncResp& operator=(AsyncResp&&) = delete;

  ~AsyncResp() { res.end(); }

  crow::response res;
};

int
main()
{
  using namespace handlers;
  try {
    // setup connection to dbus
    boost::asio::io_context io;
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);
    auto conn = std::make_shared<sdbusplus::asio::connection>(io);
    signals.async_wait([&io](const boost::system::error_code&, const int&)
                       { io.stop(); });
    crow::mustache::set_global_base("/etc/templates");

    // setup Crow
    crow::SimpleApp app;
    // run IO in detached thread
    // auto thread = std::jthread([&] { io.run(); });
    for (int i = 0; i < 1; i++) {
      workers.emplace_back([&io]() { io.run(); });
    }

    CROW_ROUTE(app, "/testxx")
    (
        [&io, &conn](crow::request& /*req*/, crow::response& res)
        {
          // AsyncResp test1 = AsyncResp(res);
          sdbusplus::asio::getProperty<double>(
              *conn,
              ServiceName,
              ObjectPath,
              InterfaceName,
              "Voltage",
              [&res](boost::system::error_code ecode, double value) mutable
              {
                using json = nlohmann::json;
                auto paramName = "Voltage";
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
                  res.sendJSON(wrapper);
                } else if (res.is_alive()) {
                  res.code = 500;
                  res.end();
                } else {
                }
              });
        });
    CROW_ROUTE(app, "/testx1")
    (
        [&io, &conn](crow::request& /*req*/, crow::response& res)
        {
          using json = nlohmann::json;
          conn->async_method_call(
              [&res](boost::system::error_code ec, std::variant<double> value)
              {
                if (ec) {
                  std::cerr << "error with async_method_call\n";
                  return;
                }
                if (auto* pval = std::get_if<double>(&value)) {
                  // std::cout << "your value is " << *pval << "\n";
                  res.code = 200;
                  json wrapper = json::object();
                  wrapper["Voltage"] = *pval;
                  res.sendJSON(wrapper);
                } else {
                  res.code = 500;
                  res.end();
                }
              },
              ServiceName,
              ObjectPath,
              FreeDeskopPath,
              "Get",
              InterfaceName,
              "Voltage");
        });

    CROW_ROUTE(app, "/testx12")
    (
        []()
        {
          crow::json::wvalue x({{"message", "Hello, World!"}});
          x["message2"] = "Hello, World.. Again!";
          return x;
        });

    // Establish routes
    CROW_ROUTE(app, "/voltage")
        .methods("GET"_method)(std::bind(
            handleVoltageRequest, conn, std::ref(io), std::placeholders::_1));

    CROW_ROUTE(app, "/test")
        .methods("GET"_method)(
            std::bind(handleTestRequest, std::placeholders::_1));

    CROW_ROUTE(app, "/data")
        .methods("GET"_method)(std::bind(
            handleDataRequest, conn, std::ref(io), std::placeholders::_1));

    CROW_ROUTE(app, "/current")
        .methods("GET"_method)(std::bind(
            handleCurrentRequest, conn, std::ref(io), std::placeholders::_1));

    CROW_ROUTE(app, "/windspeed")
        .methods("GET"_method)(std::bind(
            handleWindSpeedRequest, conn, std::ref(io), std::placeholders::_1));

    CROW_ROUTE(app, "/winddir")
        .methods("GET"_method)(std::bind(
            handleWindDirRequest, conn, std::ref(io), std::placeholders::_1));

    CROW_ROUTE(app, "/settings")
        .methods("GET"_method,
                 "PATCH"_method)(std::bind(handleSettingsRequest,
                                           conn,
                                           std::ref(io),
                                           std::placeholders::_1,
                                           std::placeholders::_2));

    CROW_ROUTE(app, "/status")
    (
        []()
        {
          crow::mustache::context ctx;
          auto page = crow::mustache::load_text("device_status.html");
          return page;
        });

    app.port(18080).multithreaded().run();
    // auto async_application = app.port(18080).run_async();
    // auto async_application = app.port(18080).multithreaded().run_async();
    //  auto async_application = app.port(18080).multithreaded().run_async();
  } catch (...) {
    std::cerr << "Error: while running the program " << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
