#pragma once

#include <chrono>
#include <ctime>
#include <format>
#include <iostream>
#include <variant>
#include <map>

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

constexpr const char* ServiceName = "setup.test.service";
constexpr const char* ObjectPath = "/top/setup/device";
constexpr const char* InterfaceName = "device.interface.test";
using variant = std::variant<int, std::string>;


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
  // setup Settings
  auto settingsName = "Settings";
  std::map<std::string, int32_t> settings{};
  iface->register_property(settingsName,
                           settings,
                           sdbusplus::asio::PropertyPermission::readWrite);
  iface->initialize();
  // run event loop
  io.run();

  return 0;
}

}  // namespace server
