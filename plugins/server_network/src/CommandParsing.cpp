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

static Parser<Package> parse_pkg()
{
  return apply([](std::uint32_t magic, ByteArray r)
               { return Package(magic, std::move(r)); },
               parseByte<std::uint32_t>(),
               parseByte<Byte>().many());
}

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

static Parser<ConnectionlessCommand> parse_connectionless()
{
  return apply([](std::uint8_t code, ByteArray c)
               { return ConnectionlessCommand(code, std::move(c)); },
               parseByte<std::uint8_t>(),
               parseByte<Byte>().many());
}

std::optional<ConnectionlessCommand>
NetworkServer::parse_connectionless_package(ByteArray& package)
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

static Parser<ConnectCommand> parse_connect_cmd()
{
  return apply(
      [](std::uint8_t p, std::uint32_t c, ByteArray name)
      { return ConnectCommand(p, c, std::string(name.begin(), name.end())); },
      parseByte<uint8_t>(),
      parseByte<u_int32_t>(),
      parseByte<Byte>() * PLAYERNAME_MAX_SIZE);
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
