#include <iostream>
#include <thread>

#include "cpu_monitor.hpp"
#include "soc_monitor.hpp"

const constexpr auto SOC_THERMAL = "/sys/class/hwmon/hwmon0/temp1_input";
int
main(int, char*[])
{
  auto threadCPU = std::thread([] { monitorCPU(); });
  auto threadSOC = std::thread([] { hwmon::runSocMonitor(SOC_THERMAL); });
  threadCPU.join();
  threadSOC.join();
  return 0;
}
