#pragma once
#include <fstream>
#include <iterator>
#include <random>
#include <sstream>
#include <utility>
#include <vector>

#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/unordered_map.hpp>

#include "base64.hpp"

namespace datamodel
{

// wind directions
constexpr short CARDINAL_DIRECTIONS = 8;
enum WindDir
{
  N,
  NE,
  E,
  SE,
  S,
  SW,
  W,
  NW
};
// types
using voltage_t = double;
using current_t = double;
using illimunation_t = double;
using winspeed_t = double;
using windirection_t = short;
using dataFormat_t = std::
    tuple<current_t, voltage_t, illimunation_t, winspeed_t, windirection_t>;

class DeviceData
{
private:
  winspeed_t windspeed {};
  illimunation_t illimination {};
  current_t current {};
  voltage_t voltage {};
  std::random_device rd;
  std::mt19937 mt;
  std::pair<current_t, current_t> currentLimits;
  std::pair<voltage_t, voltage_t> voltageLimits;
  std::pair<winspeed_t, winspeed_t> windspeedLimits;
  std::pair<illimunation_t, illimunation_t> illimLimits;
  std::pair<windirection_t, windirection_t> windDirs {
      0, CARDINAL_DIRECTIONS - 1};  // 8 sides total
  std::uniform_real_distribution<> currentDistr {currentLimits.first,
                                                 currentLimits.second};
  std::uniform_real_distribution<> voltageDistr {voltageLimits.first,
                                                 voltageLimits.second};
  std::uniform_real_distribution<> windspeedDistr {windspeedLimits.first,
                                                   windspeedLimits.second};
  std::uniform_real_distribution<> illimLimitsDistr {illimLimits.first,
                                                     illimLimits.second};
  std::uniform_int_distribution<> windDirDistr {windDirs.first,
                                                windDirs.second};

public:
  DeviceData(std::pair<current_t, current_t> currentLimits,
             std::pair<voltage_t, voltage_t> voltageLimits,
             std::pair<winspeed_t, winspeed_t> windspeedLimits,
             std::pair<illimunation_t, illimunation_t> illimLimits)
      : mt(rd())
      , currentLimits(currentLimits)
      , voltageLimits(voltageLimits)
      , windspeedLimits(windspeedLimits)
      , illimLimits(illimLimits)
  {
  }
  DeviceData() = delete;

  dataFormat_t getSampledata()
  {
    return std::make_tuple(currentDistr(mt),
                           voltageDistr(mt),
                           illimLimitsDistr(mt),
                           windspeedDistr(mt),
                           windDirDistr(mt));
  }
  voltage_t getVoltage() { return voltageDistr(mt); }
  winspeed_t getWind() { return windspeedDistr(mt); }
  illimunation_t getIlli() { return illimLimitsDistr(mt); }
  current_t getCurrent() { return currentDistr(mt); }
  current_t getWindDir() { return windDirDistr(mt); }
};
};  // namespace datamodel