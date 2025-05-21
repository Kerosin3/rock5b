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
    for (int i = 0; i < 2; i++) {
      workers.emplace_back([&io]() { io.run(); });
    }

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

    // app.port(18080).multithreaded().run();
    auto async_application = app.port(18080).run_async();
    // auto async_application = app.port(18080).run_async();
  } catch (...) {
    std::cerr << "Error: while running the program " << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
