
#include <optional>

#include "Network.hpp"
#include "ServerCommands.hpp"
#include "plugin/events/Events.hpp"

std::optional<Package> NetworkClient::parse_package(ByteArray const& package)
{
  Result<Package> r = parse_pkg()(Rest(package));

  if (r.index() == ERROR) {
    LOGGER(
        "client",
        LogLevel::ERROR,
        std::format("Failed to read package : {}", std::get<ERROR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<ConnectionlessCommand>
NetworkClient::parse_connectionless_package(ByteArray const& package)
{
  Result<ConnectionlessCommand> r = parse_connectionless()(Rest(package));

  if (r.index() == ERROR) {
    LOGGER("client",
           LogLevel::ERROR,
           std::format("Failed to read connectionless package : {}",
                       std::get<ERROR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}
