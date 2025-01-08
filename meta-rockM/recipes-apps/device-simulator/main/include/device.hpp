#pragma once
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <tuple>
#include <utility>

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

#include "datamodel.hpp"

namespace device
{
using namespace std;

using namespace std::string_literals;

constexpr const char* ServiceName = "monitoring.test.service";
constexpr const char* ObjectPath = "/top/telemetry/watcher1";
constexpr const char* InterfaceName = "watcher1.interface.test";

class DeviceData
{
  using voltage_t = double;
  using current_t = double;
  using illimunation_t = double;
  using winspeed_t = int;
  winspeed_t windspeed {};
  illimunation_t illimination {};
  current_t current {};
  voltage_t voltage {};
  std::random_device rd;
  std::mt19937 mt;
  std::pair<double, double> currentLimits;
  std::uniform_real_distribution<> currentDistr {currentLimits.first,
                                                 currentLimits.second};
  std::pair<double, double> voltageLimits;
  std::uniform_real_distribution<> voltageDistr {voltageLimits.first,
                                                 voltageLimits.second};
  std::pair<double, double> windspeedLimits;
  std::uniform_real_distribution<> windspeedDistr {windspeedLimits.first,
                                                   windspeedLimits.second};
  std::pair<double, double> illimLimits;
  std::uniform_real_distribution<> illimLimitsDistr {illimLimits.first,
                                                     illimLimits.second};
  DeviceData(std::pair<double, double> currentLimits,
             std::pair<double, double> voltageLimits,
             std::pair<double, double> windspeedLimits,
             std::pair<double, double> illimLimits)
      : mt(rd())
      , currentLimits(currentLimits)
      , voltageLimits(voltageLimits)
      , windspeedLimits(windspeedLimits)
      , illimLimits(illimLimits)
  {
  }

public:
  std::tuple<current_t, voltage_t, illimunation_t, winspeed_t> getSampledata()
  {
    return std::make_tuple(currentDistr(mt),
                           voltageDistr(mt),
                           illimLimitsDistr(mt),
                           windspeedDistr(mt));
  }
};

class Device
{
  std::mutex m_mtx;
  std::condition_variable m_cv;
  std::deque<double> m_tasks;
  // number of workers
  static constexpr ssize_t N_WORKERS = 5;
  std::jthread m_master_thread;
  std::array<std::jthread, N_WORKERS> m_worker_thread;
  // limits for voltage
  std::pair<double, double> m_limits {10.0, 14.0};
  // sampling time
  static constexpr size_t SAMPLING_TIME_MS = 1000;
  // boost members
  boost::asio::io_context io;
  std::shared_ptr<sdbusplus::asio::connection> conn;
  std::thread iothread;
  std::random_device rd;

  void spawnMaster()
  {
    m_master_thread = std::jthread([this]() { this->master(); });
  }

  void spawnWorker()
  {
    for (auto& elem : m_worker_thread) {
      elem = std::jthread([this]() { this->worker(); });
    }
  }

  auto getRandom();

  void worker();

  void addTask();

  void master();
  void setVoltage(boost::asio::yield_context yield, double voltage);

public:
  virtual ~Device() { iothread.join(); }
  Device()
  {
    conn = std::make_shared<sdbusplus::asio::connection>(io);
    iothread = std::thread([this]() { this->io.run(); });
  }
  void initialize();
};

}  // namespace device