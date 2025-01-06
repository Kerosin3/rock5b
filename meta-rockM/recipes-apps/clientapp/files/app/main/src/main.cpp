
#include <chrono>
#include <ctime>
#include <iostream>
#include <memory>
#include <thread>
#include <variant>

#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/spawn.hpp>
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

const std::string ServiceName = "xyz.openbmc_project.asio-test";
const std::string ObjectPath = "xyz/openbmc_project/test";
const std::string InterfaceName = "xyz.openbmc_project.test";
const std::string Propery = "int";

/*
void
do_start_async_todo_start_async_to_yield(std::shared_ptr<sdbusplus::asio::connection>
conn, boost::asio::yield_context yield)
{
    boost::system::error_code ec;
    setProperty(
        yield, ec, "xyz.openbmc_project.asio-test", "/xyz/openbmc_project/test",
        "xyz.openbmc_project.test", "TestFunction", int(41));

    if (!ec)
    {
        std::cout << "value is " << testValue << "\n";
    }
    else
    {
        std::cout << "ec = " << ec << ": " << testValue << "\n";
    }

}
*/

using variant = std::variant<int, std::string>;

static int var = 0;

void
do_start_async_method_call_one(
    std::shared_ptr<sdbusplus::asio::connection> conn,
    boost::asio::yield_context yield)
{
  boost::system::error_code ec;
  conn->yield_method_call<>(yield,
                            ec,
                            "xyz.openbmc_project.asio-test",
                            "/xyz/openbmc_project/test",
                            "org.freedesktop.DBus.Properties",
                            "Set",
                            "xyz.openbmc_project.test",
                            "int",
                            variant(var++));
}

void
do_start_async_method_call_two(
    std::shared_ptr<sdbusplus::asio::connection> conn,
    boost::asio::yield_context yield)
{
  boost::system::error_code ec;
  variant testValue;
  testValue =
      conn->yield_method_call<variant>(yield,
                                       ec,
                                       "xyz.openbmc_project.asio-test",
                                       "/xyz/openbmc_project/test",
                                       "org.freedesktop.DBus.Properties",
                                       "Get",
                                       "xyz.openbmc_project.test",
                                       "int");
  if (!ec) {
    std::cout << "value is " << std::get<int>(testValue) << "\n";
  }
}

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

[[nodiscard]] void
do_start_async_method_call_tree(
    std::shared_ptr<sdbusplus::asio::connection> conn,
    boost::asio::yield_context yield,
    crow::response& res)
{
  // std::shared_ptr<uint32_t> valx = std::make_shared<uint32_t>();
  sdbusplus::asio::getProperty<int32_t>(
      *conn,
      "xyz.openbmc_project.asio-test",
      "/xyz/openbmc_project/test",
      InterfaceName,
      "int",
      [&res](boost::system::error_code ec, uint32_t value) mutable
      {
        if (ec) {
          std::cout << "errror!\n";
          return;
        }
        std::cout << "Greetings value is: " << value << "\n";
        if ( res.is_alive()) {
          crow::json::wvalue x;
          x["test"] = 15;
          res.write(x.dump());
          std::cout << "exit\n";
          res.end();
        }
        res.end();
      });
  //   response["test"] = std::get<int>(testValue);
}

void
func1(crow::response& res, int after)
{
  crow::json::wvalue x;
  x["testx"] = "hahah";
  res.write("xaxaxaxax");
  res.end();
}

int
main()
{
  // setup connection to dbus
  boost::asio::io_context io;
  auto conn = std::make_shared<sdbusplus::asio::connection>(io);

  crow::SimpleApp app;
  auto thread = std::thread([&io] { io.run(); });
  thread.detach();

  CROW_ROUTE(app, "/").methods(crow::HTTPMethod::GET)(
      [&io, &conn]()
      {
        boost::asio::spawn(
            io,
            [conn](boost::asio::yield_context yield)
            { do_start_async_method_call_one(conn, yield); },
            boost::asio::detached);
        return "hello!!";
      });
  CROW_ROUTE(app, "/1").methods(crow::HTTPMethod::GET)(
      [&io, &conn]()
      {
        boost::asio::spawn(
            io,
            [conn](boost::asio::yield_context yield)
            { do_start_async_method_call_two(conn, yield); },
            boost::asio::detached);
        return "hello2!!";
      });
  CROW_ROUTE(app, "/logs")
  (
      [&io, &conn](crow::request& /*req*/, crow::response& res)
      {
        //   func1(res, after);

        boost::asio::spawn(
            io,
            [conn, &res](boost::asio::yield_context yield)
            { do_start_async_method_call_tree(conn, yield, res); },
            boost::asio::detached);
      });
  CROW_ROUTE(app, "/json")
  (
      []
      {
        crow::json::wvalue x({{"message", "Hello, World!"}});
        x["message2"] = "Hello, World.. Again!";
        return x;
      });
  app.port(18080).multithreaded().run();
}

