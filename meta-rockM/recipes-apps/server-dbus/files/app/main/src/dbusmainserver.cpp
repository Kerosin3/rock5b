#include "dbusmainserver.hpp"

namespace server
{
// timetamp
std::string
getTimestampX(void)
{
  auto time =
      std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
  return std::format("{:%Y-%m-%d %X}", time);
}

std::map<std::string, int32_t>
getParametersMap(boost::asio::yield_context /*yield*/, std::vector<uint8_t>& data)
{
  return std::map<std::string, int32_t> {};
}

std::string
encodebase64(boost::asio::yield_context /*yield*/, std::string& string_data)
{
  return base64::to_base64(string_data);
}

}  // namespace server