#include "device.hpp"

namespace device
{
using namespace datamodel;

void
Device::worker()
{
  using namespace datamodel;
  while (true) {
    std::unique_lock lock(m_mtx);
    m_cv.wait(lock, [this] { return !m_tasks.empty(); });
    auto [curr, volt, illi, wind, wdir] = m_tasks.front();
    // voltage
    boost::asio::spawn(
        io,
        [this, volt](boost::asio::yield_context yield)
        { setParameter<voltage_t>(yield, volt, "Voltage"); },
        boost::asio::detached);
    // current
    boost::asio::spawn(
        io,
        [this, curr](boost::asio::yield_context yield)
        { setParameter<current_t>(yield, curr, "Current"); },
        boost::asio::detached);
    // illi
    boost::asio::spawn(
        io,
        [this, illi](boost::asio::yield_context yield)
        { setParameter<illimunation_t>(yield, illi, "Illimunation"); },
        boost::asio::detached);
    // wind
    boost::asio::spawn(
        io,
        [this, wind](boost::asio::yield_context yield)
        { setParameter<winspeed_t>(yield, wind, "WindSpeed"); },
        boost::asio::detached);
    // wind dir
    boost::asio::spawn(
        io,
        [this, wdir](boost::asio::yield_context yield)
        { setParameter<windirection_t>(yield, wdir, "WindDir"); },
        boost::asio::detached);
    // charge
    boost::asio::spawn(
        io,
        [this](boost::asio::yield_context yield) { setCharge(yield); },
        boost::asio::detached);
    // dataPack
    boost::asio::spawn(
        io,
        [this](boost::asio::yield_context yield) { setDataPack(yield); },
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

template<typename T>
void
Device::setParameter(boost::asio::yield_context yield,
                     T param,
                     const std::string& paramName)
{
  using variant = std::variant<T, std::string>;
  boost::system::error_code ec;
  conn->yield_method_call<>(yield,
                            ec,
                            ServiceName,
                            ObjectPath,
                            FreeDeskopPath,
                            "Set",
                            InterfaceName,
                            paramName,
                            variant(param));
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
Device::setDataPack(boost::asio::yield_context yield)
{
  using variant = std::variant<double, std::string>;
  boost::system::error_code ec;
  Base64DataSamples dataToTransfer {datax.getSampledata()};
  std::stringstream ss;
  std::string encoded {};
  {
    cereal::BinaryOutputArchive oarchive(ss);  // Create an output archive

    oarchive(dataToTransfer);
    encoded = std::move(base64::to_base64(ss.str()));
  }
  conn->yield_method_call<>(yield,
                            ec,
                            ServiceName,
                            ObjectPath,
                            FreeDeskopPath,
                            "Set",
                            InterfaceName,
                            "SamplesB64",
                            variant(encoded));
}

void
Device::packAndSetData(boost::asio::yield_context yield)
{
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