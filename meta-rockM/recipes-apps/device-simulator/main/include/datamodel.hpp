#pragma once
#include <iterator>
#include <vector>

namespace data
{
struct Data
{
  virtual size_t getDataOne() = 0;
  virtual size_t getDataTwo() = 0;
  virtual size_t getDataThree() = 0;
};
};  // namespace data