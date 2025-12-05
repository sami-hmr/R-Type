#pragma once

#include <cstdint>

#include "ByteParser/ByteParser.hpp"
#include "Parser.hpp"
#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"

struct Package
{
  ByteArray magic;
  ByteArray real_package;
};

inline Parser<Package> parse_pkg()
{
  return apply([](ByteArray magic, ByteArray r)
               { return Package(std::move(magic), std::move(r)); },
               parseByte<Byte>() * 4,  // magic sequence is 4 bytes
               parseByte<Byte>().many());
}

struct ConnectionlessCommand
{
  std::uint8_t command_code;
  ByteArray command;
};

inline Parser<ConnectionlessCommand> parse_connectionless()
{
  return apply([](std::uint8_t code, ByteArray c)
               { return ConnectionlessCommand(code, std::move(c)); },
               parseByte<std::uint8_t>(),
               parseByte<Byte>().many());
}

struct ConnectCommand
{
  std::uint32_t challenge;
  std::string player_name;
};

inline Parser<ConnectCommand> parse_connect_cmd()
{
  return apply([](std::uint32_t c, std::string name)
               { return ConnectCommand(c, std::move(name)); },
               parseByte<u_int32_t>(),
               parseByteString());
}

struct ConnectResponse
{
  std::uint8_t client_id;
  std::uint32_t server_id;
};

inline Parser<ConnectResponse> parse_connect_rsp()
{
  return apply([](std::uint8_t c_id, std::uint32_t s_id)
               { return ConnectResponse(c_id, s_id); },
               parseByte<std::uint8_t>(),
               parseByte<std::uint32_t>());
}

struct ChallengeResponse {
    std::uint32_t challenge;
};

inline Parser<ChallengeResponse> parse_challenge_rsp()
{
  return apply([](std::uint32_t c)
               { return ChallengeResponse(c); },
               parseByte<std::uint32_t>());
}
