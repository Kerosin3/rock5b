#pragma once
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <mutex>
#include <thread>
#include <tuple>
#include <utility>

#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/spawn.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/tuple.hpp>
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
using namespace datamodel;

constexpr const char* ServiceName = "monitoring.test.service";
constexpr const char* ObjectPath = "/top/telemetry/watcher1";
constexpr const char* InterfaceName = "watcher1.interface.test";
constexpr const char* FreeDeskopPath = "org.freedesktop.DBus.Properties";

struct Base64DataSamples
{
  dataFormat_t data;
  Base64DataSamples() = default;

  template<typename... U>
  Base64DataSamples(U... args)
      : data {std::make_tuple((args)...)}
  {
  }

  Base64DataSamples(dataFormat_t&& datain)
      : data {std::move(datain)}
  {
  }

  template<typename Archive>
  void serialize(Archive& archive)
  {
    archive(data);
  }
};

class Device
{
  std::mutex m_mtx;
  std::condition_variable m_cv;
  std::deque<dataFormat_t> m_tasks;
  // number of workers
  static constexpr ssize_t N_WORKERS = 5;
  std::jthread m_master_thread;
  std::array<std::jthread, N_WORKERS> m_worker_thread;
  // sampling time
  static constexpr size_t SAMPLING_TIME_MS = 1000;
  // boost members
  boost::asio::io_context io;
  std::shared_ptr<sdbusplus::asio::connection> conn;
  std::thread iothread;
  std::random_device rd;
  DeviceData datax {std::make_pair(0.0, 5.0),
                    std::make_pair(10.0, 15.0),
                    std::make_pair(-30.0, 30.0),
                    std::make_pair(0.0, 100.0)};

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

  void worker();

  void addTask();

  void master();

  template<typename T>
  void setParameter(boost::asio::yield_context yield,
                    T param,
                    const std::string& paramName);

  void setCharge(boost::asio::yield_context yield);

  void setDataPack(boost::asio::yield_context yield);

  void packAndSetData(boost::asio::yield_context yield);

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