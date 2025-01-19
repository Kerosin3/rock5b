#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace hwmon
{

int64_t
readTemp(const std::string& path)
{
  int64_t val;
  std::ifstream ifs;
  ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit
                 | std::ifstream::eofbit);

  errno = 0;
  if (!ifs.is_open())
    ifs.open(path);
  ifs.clear();
  ifs.seekg(0);
  ifs >> val;

  return val;
}

double
getSocTemp(const std::string& path)
{
  auto temp = static_cast<float>(readTemp(path));
  return (temp / 1000.0);
}

void
runSocMonitor(const std::string& path)
{
  for (;;) {
    auto temp = static_cast<float>(readTemp(path));
    std::cout << "temp is " << (temp / 1000.0) << "\n";
    sleep(1);
  }
}
};  // namespace hwmon