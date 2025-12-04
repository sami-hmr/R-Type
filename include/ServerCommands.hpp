#pragma once

#include <cstdint>
#include "ByteParser/ByteParser.hpp"
#include "Parser.hpp"
#include "plugin/Byte.hpp"

struct Package {
    ByteArray magic;
    ByteArray real_package;
};

inline Parser<Package> parse_pkg()
{
  return apply([](ByteArray magic, ByteArray r)
               { return Package(std::move(magic), std::move(r)); },
               parseByte<Byte>() * 4, // magic sequence is 4 bytes
               parseByte<Byte>().many());
}

struct ConnectionlessCommand {
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

struct ConnectCommand {
    std::uint32_t challenge;
    std::string player_name;
};

inline Parser<ConnectCommand> parse_connect_cmd()
{
  return apply(
      [](std::uint32_t c, std::string name)
      { return ConnectCommand(c, std::move(name)); },
      parseByte<u_int32_t>(),
      parseByteString());
}
