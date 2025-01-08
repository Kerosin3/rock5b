#include <chrono>
#include <ctime>
#include <format>
#include <iostream>
#include <variant>

#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/asio/sd_event.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/timer.hpp>

namespace server
{

constexpr const char* ServiceName = "monitoring.test.service";
constexpr const char* ObjectPath = "/top/telemetry/watcher1";
constexpr const char* InterfaceName = "watcher1.interface.test";

using variant = std::variant<int, std::string>;

std::string
getTimestampX(void)
{
  auto time =
      std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
  return std::format("{:%Y-%m-%d %X}", time);
}

auto
getParameters(boost::asio::yield_context /*yield*/, std::vector<uint8_t>& data)
{
  return std::map<std::string, variant> {};
}

int
server()
{
  boost::asio::io_context io;
  auto conn = std::make_shared<sdbusplus::asio::connection>(io);
  // test object server
  conn->request_name(ServiceName);
  // establish connection
  auto server = sdbusplus::asio::object_server(conn);
  // add interface
  std::shared_ptr<sdbusplus::asio::dbus_interface> iface =
      server.add_interface(ObjectPath, InterfaceName);
  // add Date
  auto properyDateName = "CurrentDate";
  std::string currentDate = {"dasdasd"};

  iface->register_property(properyDateName,
                           currentDate,
                           sdbusplus::asio::PropertyPermission::readWrite);
  // current in amps

  auto properyCurrentName = "Current";
  double currentDataAmps {0.0};
  iface->register_property(properyCurrentName,
                           currentDataAmps,
                           sdbusplus::asio::PropertyPermission::readWrite);
  // voltage
  auto properyVoltageName = "Voltage";
  double voltageData {0.0};
  iface->register_property(properyVoltageName,
                           voltageData,
                           [](const int& voltage, double& propertyValue)
                           {
                             if (voltage < 0.0) {
                               return false;
                             }
                             propertyValue = voltage;
                             return true;
                           });
  // Wind speed
  auto properyWindspeedName = "WindSpeed";
  int32_t currentWindspeed {0};
  iface->register_property(properyWindspeedName,
                           currentWindspeed,
                           sdbusplus::asio::PropertyPermission::readWrite);
  // Uptime
  auto properyUptimeName = "Uptime";
  int32_t uptimeMin {0};
  iface->register_property(properyUptimeName,
                           uptimeMin,
                           sdbusplus::asio::PropertyPermission::readWrite);
  iface->register_method("getTimestamp", getTimestampX);
  iface->register_method("toMap", getParameters);

  iface->initialize();
  io.run();

  return 0;
}

}  // namespace server

int
main(int argc, const char* argv[])
{
  server::server();
  return -1;
}
