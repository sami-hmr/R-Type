// #include <cstdint>
// #include <optional>
// #include <vector>

#include <optional>

#include <sys/types.h>

#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "Parser.hpp"
#include "ParserTypes.hpp"
#include "Rest.hpp"
#include "ServerCommands.hpp"
#include "network/server/Server.hpp"
#include "plugin/Byte.hpp"

std::optional<Package> Server::parse_package(ByteArray const& package)
{
  Result<Package> r = parse_pkg()(Rest(package));

  if (r.index() == ERR) {
    LOGGER_EVTLESS(
        LogLevel::ERROR,
        "server",
        std::format("Failed to read package : {}", std::get<ERR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<ConnectionlessCommand> Server::parse_connectionless_package(
    ByteArray const& package)
{
  Result<ConnectionlessCommand> r = parse_connectionless()(Rest(package));

  if (r.index() == ERR) {
    LOGGER_EVTLESS(LogLevel::ERROR,
                   "server",
                   std::format("Failed to read connectionless package : {}",
                               std::get<ERR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<ConnectCommand> Server::parse_connect_command(
    ByteArray const& package)
{
  Result<ConnectCommand> r = parse_connect_cmd()(Rest(package));

  if (r.index() == ERR) {
    LOGGER_EVTLESS(LogLevel::ERROR,
                   "server",
                   std::format("Failed to read connect command : {}",
                               std::get<ERR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<ConnectedPackage> Server::parse_connected_package(
    ByteArray const& package)
{
  Result<ConnectedPackage> r = parse_connected()(Rest(package));

  if (r.index() == ERR) {
    LOGGER_EVTLESS(LogLevel::ERROR,
                   "server",
                   std::format("Failed to read connected package : {}",
                               std::get<ERR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<ConnectedCommand> Server::parse_connected_command(
    ByteArray const& package)
{
  Result<ConnectedCommand> r = parse_connected_cmd()(package);

  if (r.index() == ERR) {
    LOGGER_EVTLESS(LogLevel::ERROR,
                   "server",
                   std::format("Failed to read connected command : {}",
                               std::get<ERR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<EventBuilder> Server::parse_event_build_cmd(
    ByteArray const& package)
{
  Result<EventBuilder> r = parse_event_builder()(package);

  if (r.index() == ERR) {
    LOGGER_EVTLESS(LogLevel::ERROR,
                   "server",
                   std::format("Failed to read event command : {}",
                               std::get<ERR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<ComponentBuilder> Server::parse_component_build_cmd(
    ByteArray const& package)
{
  Result<ComponentBuilder> r = parse_component_builder()(package);

  if (r.index() == ERR) {
    LOGGER_EVTLESS(LogLevel::ERROR,
                   "server",
                   std::format("Failed to read component command : {}",
                               std::get<ERR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<HearthBeat> Server::parse_hearthbeat_cmd(ByteArray const& package)
{
  try {
    return HearthBeat(package);
  } catch (InvalidPackage const& e) {
    LOGGER_EVTLESS(
        LogLevel::ERROR,
        "server",
        std::format("Failed to read component command : {}", e.what()));
  }
  return std::nullopt;
}
