#pragma once

#include <cstdint>
#include "plugin/Byte.hpp"

struct Package {
    std::uint32_t magic;
    ByteArray real_package;
};

struct ConnectionlessCommand {
    std::uint8_t command_code;
    ByteArray command;
};

struct ConnectCommand {
    std::uint8_t protocol;
    std::uint32_t challenge;
    std::string player_name;
};
