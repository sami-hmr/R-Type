
#include <optional>

#include "ParserTypes.hpp"
#include "ServerCommands.hpp"
#include "network/client/Client.hpp"

std::optional<Package> Client::parse_package(ByteArray const& package)
{
  Result<Package> r = parse_pkg()(Rest(package));

  if (r.index() == ERROR) {
    NETWORK_LOGGER(
        "client",
        LogLevel::ERROR,
        std::format("Failed to read package : {}", std::get<ERROR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<ConnectionlessCommand> Client::parse_connectionless_package(
    ByteArray const& package)
{
  Result<ConnectionlessCommand> r = parse_connectionless()(Rest(package));

  if (r.index() == ERROR) {
    NETWORK_LOGGER("client",
                   LogLevel::ERROR,
                   std::format("Failed to read connectionless package : {}",
                               std::get<ERROR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<ConnectResponse> Client::parse_connect_response(
    ByteArray const& package)
{
  Result<ConnectResponse> r = parse_connect_rsp()(package);

  if (r.index() == ERROR) {
    NETWORK_LOGGER("client",
                   LogLevel::ERROR,
                   std::format("Failed to read connect response package : {}",
                               std::get<ERROR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<ChallengeResponse> Client::parse_challenge_response(
    ByteArray const& package)
{
  Result<ChallengeResponse> r = parse_challenge_rsp()(package);

  if (r.index() == ERROR) {
    NETWORK_LOGGER("client",
                   LogLevel::ERROR,
                   std::format("Failed to read challenge response package : {}",
                               std::get<ERROR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<ConnectedPackage> Client::parse_connected_package(
    ByteArray const& package)
{
  Result<ConnectedPackage> r = parse_connected()(Rest(package));

  if (r.index() == ERROR) {
    NETWORK_LOGGER("client",
                   std::uint8_t(LogLevel::ERROR),
                   std::format("Failed to read connected package : {}",
                               std::get<ERROR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<ConnectedCommand> Client::parse_connected_command(
    ByteArray const& package)
{
  Result<ConnectedCommand> r = parse_connected_cmd()(package);

  if (r.index() == ERROR) {
    NETWORK_LOGGER("server",
                   std::uint8_t(LogLevel::ERROR),
                   std::format("Failed to read connected command : {}",
                               std::get<ERROR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<EventBuilder> Client::parse_event_build_cmd(
    ByteArray const& package)
{
  Result<EventBuilder> r = parse_event_builder()(package);

  if (r.index() == ERROR) {
    NETWORK_LOGGER("server",
                   std::uint8_t(LogLevel::ERROR),
                   std::format("Failed to read event command : {}",
                               std::get<ERROR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<ComponentBuilder> Client::parse_component_build_cmd(
    ByteArray const& package)
{
  Result<ComponentBuilder> r = parse_component_builder()(package);

  if (r.index() == ERROR) {
    NETWORK_LOGGER("server",
                   std::uint8_t(LogLevel::ERROR),
                   std::format("Failed to read component command : {}",
                               std::get<ERROR>(r).message));
    return std::nullopt;
  }
  return std::get<SUCCESS>(r).value;
}

std::optional<HearthBeat> Client::parse_hearthbeat_cmd(ByteArray const& package)
{
  try {
    return HearthBeat(package);
  } catch (InvalidPackage const& e) {
    NETWORK_LOGGER(
        "server",
        LogLevel::ERROR,
        std::format("Failed to read component command : {}", e.what()));
  }
  return std::nullopt;
}
