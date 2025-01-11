#include <chrono>
#include <ctime>
#include <format>
#include <fstream>
#include <iostream>
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

#include "device.hpp"
/*
using variant = std::variant<int, std::string>;

static int var = 15;


constexpr const char* ServiceName = "monitoring.test.service";
constexpr const char* ObjectPath = "/top/telemetry/watcher1";
constexpr const char* InterfaceName = "watcher1.interface.test";


void
do_start_async_method_call_one(
    std::shared_ptr<sdbusplus::asio::connection> conn,
    boost::asio::yield_context yield)
{
  boost::system::error_code ec;
  int var1 {126};
  conn->yield_method_call<>(yield,
                            ec,
                            "xyz.openbmc_project.asio-test",
                            "/xyz/openbmc_project/test",
                            "org.freedesktop.DBus.Properties",
                            "Set",
                            "xyz.openbmc_project.test",
                            "int",
                            variant(var1));
  std::cout << "exec!\n";
}
*/
/*
void
setVoltage(std::shared_ptr<sdbusplus::asio::connection> conn,
           boost::asio::yield_context yield)
{
  boost::system::error_code ec;
  int var1 {133};
  conn->yield_method_call<>(yield,
                            ec,
                            ServiceName,
                            ObjectPath,
                            "org.freedesktop.DBus.Properties",
                            "Set",
                            "watcher1.interface.test",
                            "Voltage",
                            variant(var1));
  std::cout << "exec!\n";
}
*/
/*
void
do_start_async_method_call_two(
    std::shared_ptr<sdbusplus::asio::connection> conn,
    boost::asio::yield_context yield)
{
  using variant = std::variant<int, std::string>;
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
*/
int
main(int argc, const char* argv[])
{
  device::Device device {};
  device.initialize();
  std::cout << "START APP\n";
  return 0;
}
