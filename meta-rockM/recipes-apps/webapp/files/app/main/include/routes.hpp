#pragma once

#include <memory>

#include <boost/asio/io_context.hpp>
#include <sdbusplus/asio/connection.hpp>

#include "webapp.hpp"

namespace webroutes
{
auto
test_route(const crow::request& /*req*/,
           const crow::response& res,
           std::shared_ptr<sdbusplus::asio::connection> conn,
           boost::asio::io_context& io)
{
//   [&io, &conn]() {};

  return 5;
}

}  // namespace webroutes