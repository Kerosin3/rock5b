#include "device.hpp"

namespace device
{
using namespace datamodel;

void
Device::worker()
{
  while (true) {
    std::unique_lock lock(m_mtx);
    m_cv.wait(lock, [this] { return !m_tasks.empty(); });
    auto [curr, volt, illi, wind, wdir] = m_tasks.front();
    // voltage
    boost::asio::spawn(
        io,
        [this, volt](boost::asio::yield_context yield)
        { setVoltage(yield, volt); },
        boost::asio::detached);
    // current
    boost::asio::spawn(
        io,
        [this, curr](boost::asio::yield_context yield)
        { setCurrent(yield, curr); },
        boost::asio::detached);
    // illi
    boost::asio::spawn(
        io,
        [this, illi](boost::asio::yield_context yield)
        { setIllim(yield, illi); },
        boost::asio::detached);
    // wind
    boost::asio::spawn(
        io,
        [this, wind](boost::asio::yield_context yield)
        { setWindspeed(yield, wind); },
        boost::asio::detached);
    // wind dir
    boost::asio::spawn(
        io,
        [this, wdir](boost::asio::yield_context yield)
        { setWindDir(yield, wdir); },
        boost::asio::detached);
    // charge
    boost::asio::spawn(
        io,
        [this](boost::asio::yield_context yield)
        { setCharge(yield); },
        boost::asio::detached);
    m_tasks.pop_front();
    // sampling time
    std::this_thread::sleep_for(std::chrono::milliseconds(SAMPLING_TIME_MS));
    lock.unlock();
    m_cv.notify_all();
  }
}

void
Device::master()
{
  while (true) {
    {
      std::lock_guard lock(m_mtx);
      addTask();
      // for tests std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    m_cv.notify_all();
    {
      std::unique_lock lock(m_mtx);
      m_cv.wait(lock, [this] { return m_tasks.empty(); });
    }
  }
}

void
Device::setVoltage(boost::asio::yield_context yield, voltage_t voltage)
{
  using variant = std::variant<voltage_t, std::string>;
  boost::system::error_code ec;
  conn->yield_method_call<>(yield,
                            ec,
                            ServiceName,
                            ObjectPath,
                            FreeDeskopPath,
                            "Set",
                            InterfaceName,
                            "Voltage",
                            variant(voltage));
}

void
Device::setCurrent(boost::asio::yield_context yield, current_t current)
{
  using variant = std::variant<current_t, std::string>;
  boost::system::error_code ec;
  conn->yield_method_call<>(yield,
                            ec,
                            ServiceName,
                            ObjectPath,
                            FreeDeskopPath,
                            "Set",
                            InterfaceName,
                            "Current",
                            variant(current));
}
void
Device::setIllim(boost::asio::yield_context yield, illimunation_t illim)
{
  using variant = std::variant<illimunation_t, std::string>;
  boost::system::error_code ec;
  conn->yield_method_call<>(yield,
                            ec,
                            ServiceName,
                            ObjectPath,
                            FreeDeskopPath,
                            "Set",
                            InterfaceName,
                            "Illimunation",
                            variant(illim));
}
void
Device::setWindspeed(boost::asio::yield_context yield, winspeed_t windspped)
{
  using variant = std::variant<winspeed_t, std::string>;
  boost::system::error_code ec;
  conn->yield_method_call<>(yield,
                            ec,
                            ServiceName,
                            ObjectPath,
                            FreeDeskopPath,
                            "Set",
                            InterfaceName,
                            "WindSpeed",
                            variant(windspped));
}
void
Device::setCharge(boost::asio::yield_context yield)
{
  using variant = std::variant<double, std::string>;
  boost::system::error_code ec;
  static std::mt19937 mt(rd());
  std::uniform_real_distribution uniform(0.0, 100.0);
  conn->yield_method_call<>(yield,
                            ec,
                            ServiceName,
                            ObjectPath,
                            FreeDeskopPath,
                            "Set",
                            InterfaceName,
                            "Charge",
                            variant(uniform(mt)));
}
void
Device::setWindDir(boost::asio::yield_context yield, windirection_t wdir)
{
  using variant = std::variant<windirection_t, std::string>;
  boost::system::error_code ec;
  conn->yield_method_call<>(yield,
                            ec,
                            ServiceName,
                            ObjectPath,
                            FreeDeskopPath,
                            "Set",
                            InterfaceName,
                            "WindDir",
                            variant(wdir));
}
void
Device::initialize()
{
  spawnMaster();
  spawnWorker();
}

void
Device::addTask()
{
  auto data = datax.getSampledata();
  m_tasks.emplace_front(data);
}
}  // namespace device