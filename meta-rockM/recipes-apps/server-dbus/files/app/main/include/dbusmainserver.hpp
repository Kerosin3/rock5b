#pragma once

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

#include "base64.hpp"

namespace server
{

constexpr const char* ServiceName = "monitoring.test.service";
constexpr const char* ObjectPath = "/top/telemetry/watcher1";
constexpr const char* InterfaceName = "watcher1.interface.test";
using variant = std::variant<int, std::string>;

std::string
getTimestampX(void);

std::map<std::string, int32_t>
getParametersMap(boost::asio::yield_context /*yield*/,
                 std::vector<uint8_t>& data);

std::string
encodebase64(boost::asio::yield_context /*yield*/, std::string& string_data);

inline int
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
                           [](const double& voltage, double& propertyValue)
                           {
                             if (voltage < 0.0) {
                               return false;
                             }
                             propertyValue = voltage;
                             return true;
                           });
  // Wind direction
  auto windDirName = "WindDir";
  short currentWindDir {0};
  iface->register_property(windDirName,
                           currentWindDir,
                           sdbusplus::asio::PropertyPermission::readWrite);
  // Wind speed
  auto properyWindspeedName = "WindSpeed";
  double currentWindspeed {0};
  iface->register_property(properyWindspeedName,
                           currentWindspeed,
                           sdbusplus::asio::PropertyPermission::readWrite);
  // Illim
  auto properyIllimunationName = "Illimunation";
  double currentIllim {0};
  iface->register_property(properyIllimunationName,
                           currentIllim,
                           sdbusplus::asio::PropertyPermission::readWrite);
  // Uptime
  auto properyUptimeName = "Uptime";
  int32_t uptimeMin {0};
  iface->register_property(properyUptimeName,
                           uptimeMin,
                           sdbusplus::asio::PropertyPermission::readWrite);
  // Charged percentage
  auto propertyChargedName = "Charge";
  double charged {0};
  iface->register_property(propertyChargedName,
                           charged,
                           [](const double& charge, double& propertyValue)
                           {
                             if ((0.0 < charge) && (charge <= 100.0)) {
                               propertyValue = charge;
                               return true;
                             }
                             return false;
                           });
  // cpuload
  auto CpuLoadName = "cpuUsage";
  int32_t cpuUsage {0};
  iface->register_property(
      CpuLoadName, cpuUsage, sdbusplus::asio::PropertyPermission::readWrite);
  // register methods
  iface->register_method("getTimestamp", getTimestampX);
  iface->register_method("encodebase64", encodebase64);
  iface->register_method("getParameters", getParametersMap);

  iface->initialize();
  // run event loop
  io.run();

  return 0;
}

}  // namespace server
