#include "device.hpp"

namespace device
{
using namespace std;

using namespace std::string_literals;

void
Device::worker()
{
  while (true) {
    std::unique_lock lock(m_mtx);
    m_cv.wait(lock, [this] { return !m_tasks.empty(); });
    // std::cout << std::this_thread::get_id() << " value is " << m_tasks.front()
              // << "\n";
    boost::asio::spawn(
        io,
        [this](boost::asio::yield_context yield)
        { setVoltage(yield, m_tasks.front()); },
        boost::asio::detached);
    m_tasks.pop_front();
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
      // std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    m_cv.notify_all();
    {
      std::unique_lock lock(m_mtx);
      m_cv.wait(lock, [this] { return m_tasks.empty(); });
    }
  }
}

void
Device::setVoltage(boost::asio::yield_context yield, double voltage)
{
  using variant = std::variant<double, std::string>;
  boost::system::error_code ec;
  conn->yield_method_call<>(yield,
                            ec,
                            ServiceName,
                            ObjectPath,
                            "org.freedesktop.DBus.Properties",
                            "Set",
                            InterfaceName,
                            "Voltage",
                            variant(voltage));
}

void
Device::initialize()
{
  spawnMaster();
  spawnWorker();
}
auto
Device::getRandom()
{
  static std::mt19937 mt(rd());
  std::uniform_real_distribution uniform(m_limits.first, m_limits.second);
  return uniform(mt);
}

void
Device::addTask()
{
  m_tasks.emplace_front(getRandom());
}
}  // namespace device