#include <cstdint>
#include <optional>
#include <vector>

#include <sys/types.h>

#include "ByteParser/ByteParser.hpp"
#include "Network.hpp"
#include "Parser.hpp"
#include "ParserTypes.hpp"
#include "ParserUtils.hpp"
#include "Rest.hpp"
#include "ServerCommands.hpp"
#include "plugin/Byte.hpp"
#include "plugin/events/Events.hpp"



std::optional<Package> NetworkServer::parse_package(ByteArray const& package)
{
  Result<Package> r = parse_pkg()(Rest(package));

  if (r.index() == ERROR) {
    LOGGER(
        "server",
        LogLevel::ERROR,
        std::format("Failed to read package : {}", std::get<ERROR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<ConnectionlessCommand>
NetworkServer::parse_connectionless_package(ByteArray const& package)
{
  Result<ConnectionlessCommand> r = parse_connectionless()(Rest(package));

  if (r.index() == ERROR) {
    LOGGER("server",
           LogLevel::ERROR,
           std::format("Failed to read connectionless package : {}",
                       std::get<ERROR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<ConnectCommand> NetworkServer::parse_connect_command(
    ByteArray const& cmd)
{
  Result<ConnectCommand> r = parse_connect_cmd()(Rest(cmd));

  if (r.index() == ERROR) {
    LOGGER("server",
           LogLevel::ERROR,
           std::format("Failed to read connect command : {}",
                       std::get<ERROR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}
