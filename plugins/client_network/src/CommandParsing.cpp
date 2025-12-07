
#include <optional>

#include "Client.hpp"
#include "ParserTypes.hpp"
#include "ServerCommands.hpp"
#include "plugin/events/Events.hpp"

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

std::optional<ConnectionlessCommand>
Client::parse_connectionless_package(ByteArray const& package)
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

std::optional<ConnectResponse> Client::parse_connect_response(ByteArray const& package) {
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

std::optional<ChallengeResponse> Client::parse_challenge_response(ByteArray const& package) {
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
