#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <string>

#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>
#include "ServerCommands.hpp"
#include "plugin/Byte.hpp"

#define MAX_PLAYERS 4
#define BUFFER_SIZE 2048

#define NETWORK_LOGGER(category, level, message) \
  std::cerr  << (category) << ": " << (message) << "\n"

#define __VERSION_MAGIC_SEQUENCE__ 0x436482793
//#define MAGIC_SEQUENCE std::uint32_t(0x436482793)

consteval std::array<Byte, 4> __generate_index_sequence__() {
    auto magic = static_cast<uint32_t>(__VERSION_MAGIC_SEQUENCE__);
    return std::bit_cast<std::array<Byte, 4>>(magic);
}

static constexpr std::array<Byte, 4> __MAGIC_SEQUENCE__ = __generate_index_sequence__();


static const ByteArray MAGIC_SEQUENCE = {__MAGIC_SEQUENCE__.begin(), __MAGIC_SEQUENCE__.end()};
static const ByteArray PROTOCOL_EOF = {0x67, 0x67, 0x67, 0x67};

#define HOSTNAME_LENGTH 64
#define MAPNAME_LENGTH 32
#define PLAYERNAME_MAX_SIZE 32
#define ERROR_MSG_SIZE 32

#define CHALLENGE_SIZE 4
#define SERVER_ID_SIZE 4
#define CLIENT_ID_SIZE 1
#define PROTOCOL_SIZE 1
#define COMMAND_SIZE 1
#define CONNECT_COMMAND_SIZE 37

#define DISCONNECT_RESP_CMD_SIZE 2
#define CHALLENGE_RESP_CMD_SIZE 2
#define CONNECT_RESP_CMD_SIZE 3
#define CONNECT_CMD_SIZE 3

#define SLEEP_DURATION 10
#define UNUSED __attribute__((unused))

#define CURRENT_PROTOCOL_VERSION std::uint8_t(1)
#define SOLO uint8_t(0x01)
#define COOP uint8_t(0x02)

#define END_OF_CMD std::uint8_t(0x00)

#define CMD_INDEX 0
#define PROTOCOL_INDEX 0

#define CHALLENGE_CLG_INDEX 1

#define ERR_MESS_DSCNT_INDEX 1

#define CHALLENGE_CNT_INDEX 1
#define PLAYERNAME_CNT_INDEX 2

#define CLIENT_ID_CNT_RESP_INDEX 1
#define SERVER_ID_CNT_RESP_INDEX 2

enum DisconnectedCommands : std::uint8_t
{
  GETINFO = 0x01,
  GETSTATUS,
  GETCHALLENGE,
  CONNECT,
  INFORESPONSE,
  STATUSRESPONSE,
  CHALLENGERESPONSE,
  CONNECTRESPONSE,
  DISCONNECT
};


enum ConnectedOpcodes : std::uint8_t
{
    SENDEVENT = 0x01,
    SENDCOMP = 0x02,
    ENTITYCREATION = 0x05,
};

enum class ConnectionState : std::uint8_t
{
  DISCONNECTED = 0,
  CHALLENGING,
  CONNECTING,
  CONNECTED
};

enum class ClientState : std::uint8_t
{
  DISCONNECTED = 0,
  CHALLENGING,
  CONNECTED
};

struct ClientInfo
{
  asio::ip::udp::endpoint endpoint;
  std::string player_name;
  ClientState state = ClientState::DISCONNECTED;
  std::uint32_t last_sequence = 0;
  std::uint32_t challenge = 0;
  std::uint8_t client_id = 0;
  std::uint32_t score = 0;
  std::uint8_t ping = 0;
};
