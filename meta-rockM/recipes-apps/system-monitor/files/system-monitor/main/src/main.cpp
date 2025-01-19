#include <chrono>
#include <ctime>
#include <format>
#include <iostream>
#include <variant>

#include <boost/asio.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/bind.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/asio/sd_event.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/timer.hpp>

#include "cpu_monitor.hpp"
#include "soc_monitor.hpp"
#include "uptime.hpp"

const constexpr auto SOC_THERMAL = "/sys/class/hwmon/hwmon0/temp1_input";
constexpr const char* ServiceName = "monitoring.test.service";
constexpr const char* ObjectPath = "/top/telemetry/watcher1";
constexpr const char* InterfaceName = "watcher1.interface.test";
constexpr const char* FreeDeskopPath = "org.freedesktop.DBus.Properties";

void
print(const boost::system::error_code& /*e*/)
{
  std::cout << "Hello, world!\n";
}

int
main(int, char*[])
{
  boost::asio::io_context io;
  auto conn = std::make_shared<sdbusplus::asio::connection>(io);
  auto thread = std::jthread([&] { io.run(); });
  // set Temperature
  boost::asio::spawn(
      io,
      [&](boost::asio::yield_context yield)
      {
        for (;;) {
          boost::asio::system_timer timer(io);
          timer.expires_from_now(std::chrono::seconds(1));
          timer.async_wait(yield);
          double socTemp = hwmon::getSocTemp(SOC_THERMAL);
          using variant = std::variant<double, std::string>;
          boost::system::error_code ec;
          conn->yield_method_call<>(yield,
                                    ec,
                                    ServiceName,
                                    ObjectPath,
                                    FreeDeskopPath,
                                    "Set",
                                    InterfaceName,
                                    "SocTemp",
                                    variant(socTemp));
        }
      },
      boost::asio::detached);
  // set Uptime
  boost::asio::spawn(
      io,
      [&](boost::asio::yield_context yield)
      {
        for (;;) {
          boost::asio::system_timer timer(io);
          timer.expires_from_now(std::chrono::seconds(10));
          timer.async_wait(yield);

          int64_t uptime = get_uptime();
          using variant = std::variant<int64_t, std::string>;
          boost::system::error_code ec;
          conn->yield_method_call<>(yield,
                                    ec,
                                    ServiceName,
                                    ObjectPath,
                                    FreeDeskopPath,
                                    "Set",
                                    InterfaceName,
                                    "Uptime",
                                    variant(uptime));
        }
      },
      boost::asio::detached);
  // set CpuUsage
  boost::asio::spawn(
      io,
      [&](boost::asio::yield_context yield)
      {
        for (;;) {
          using variant = std::variant<double, std::string>;
          size_t previous_idle_time = 0, previous_total_time = 0;
          for (size_t idle_time, total_time;
               get_cpu_times(idle_time, total_time);) {
            boost::asio::system_timer timer(io);
            timer.expires_from_now(std::chrono::seconds(1));
            timer.async_wait(yield);

            const float idle_time_delta = idle_time - previous_idle_time;
            const float total_time_delta = total_time - previous_total_time;
            double workload =
                100.0 * (1.0 - idle_time_delta / total_time_delta);
            previous_idle_time = idle_time;
            previous_total_time = total_time;
            boost::system::error_code ec;
              conn->yield_method_call<>(yield,
                                        ec,
                                        ServiceName,
                                        ObjectPath,
                                        FreeDeskopPath,
                                        "Set",
                                        InterfaceName,
                                        "CpuUsage",
                                        variant(workload));
          }
        }
      },
      boost::asio::detached);

  return 0;
}
