#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <map>
#include <variant>

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/bus/match.hpp>

#include "settings.hpp"

constexpr auto fooProperty = "Settings";
constexpr auto service = "setup.test.service";
constexpr auto path = "/top/setup/device";
constexpr auto interface = "device.interface.test";
constexpr const char* FreeDeskopPath = "org.freedesktop.DBus.Properties";

constexpr auto cfgfile = "/etc/config.yaml";

boost::asio::io_context io;
auto conn = std::make_shared<sdbusplus::asio::connection>(io);

std::optional<sdbusplus::bus::match_t> match;

void
registerMatch()
{
  match = sdbusplus::bus::match_t(
      *conn,
      sdbusplus::bus::match::rules::propertiesChanged(path, interface),
      [](sdbusplus::message_t& message)
      {
        std::string iface;
        using variant =
            std::variant<std::map<std::string, int32_t>, std::string>;
        std::map<std::string, variant> properties;
        message.read(iface, properties);
        auto pgoodProp = properties.find(fooProperty);
        if (pgoodProp != properties.end()) {
          Smanager::Config cfg1(cfgfile);
          auto pgood =
              std::get<std::map<std::string, int32_t>>(pgoodProp->second);
          // update settings
          std::map<std::string, std::variant<std::string, std::int32_t>>
              nmap {};
          for (auto& [k, v] : pgood) {
            // std::cout << "settingname " << k << " value: " << v << "\n";
            nmap[k] = v;
          }
          cfg1.writeSettings("settings", nmap);
        }
      });
}

void
timerHandler()
{
  registerMatch();
}

int
main()
{
  if (!std::filesystem::exists(cfgfile)) {
    std::cerr << "config file not exists!\n";
    return EXIT_FAILURE;
  }
  sdbusplus::asio::object_server objectServer(conn);

  // read current setup
  {
    boost::asio::spawn(
        io,
        [](boost::asio::yield_context yield)
        {
          Smanager::Config cfg1(cfgfile);
          std::map<std::string, int32_t>
              readed_settings = cfg1.processSettings();

          using variant = std::variant<
              std::map<std::string, int32_t>>;
          boost::system::error_code ec;
          conn->yield_method_call<>(yield,
                                    ec,
                                    service,
                                    path,
                                    FreeDeskopPath,
                                    "Set",
                                    interface,
                                    "Settings",
                                    variant(readed_settings));
        },

        boost::asio::detached);
  }
  // retup handler
  timerHandler();

  io.run();

  return 0;
}
