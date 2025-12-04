#pragma once

#include <array>
#include <cstdint>
// #include <functional>
// #include <map>
// #include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include <boost/system/error_code.hpp>

#include "ServerLaunch.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/CircularBuffer.hpp"
#include "plugin/EntityLoader.hpp"
// #include "plugin/events/Events.hpp"

#define MAX_PLAYERS 4

#define BUFFER_SIZE 2048

#define MAGIC_SEQUENCE 0x67676767
#define MAGIC_LENGTH 4

#define HOSTNAME_LENGTH 64
#define MAPNAME_LENGTH 32

#define SLEEP_DURATION 10

#define UNUSED __attribute__((unused))

#define CURRENT_PROTOCOL_VERSION 1

#define SOLO uint8_t(0x01)
#define COOP uint8_t(0x02)

#define CONNECT_CMD_SIZE 3

#define END_OF_CMD std::uint8_t(0x00)

// indexes

#define CMD_INDEX 0

#define PROTOCOL_INDEX 0

#define CHALLENGE_CNT_INDEX 1
#define PLAYERNAME_CNT_INDEX 2

// size in byte

#define CONNECT_COMMAND_SIZE 37
#define PLAYERNAME_MAX_SIZE 32
#define ERROR_MSG_SIZE 32
#define CHALLENGE_SIZE 4
#define PROTOCOL_SIZE 1
#define COMMAND_SIZE 1

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

enum class ClientState : std::uint8_t
{
  DISCONNECTED = 0,
  CHALLENGING,
  CONNECTED
};

struct ClientInfo
{
  asio::ip::udp::endpoint endpoint;
  std::array<char, PLAYERNAME_MAX_SIZE> player_name {};
  ClientState state = ClientState::DISCONNECTED;
  std::uint32_t last_sequence = 0;
  std::uint32_t challenge = 0;
  std::uint8_t client_id = 0;
  std::uint32_t score = 0;
  std::uint8_t ping = 0;
};

class NetworkServer : public APlugin
{
public:
  NetworkServer(Registery& r, EntityLoader& l);
  ~NetworkServer() override;

private:
  void launch_server(ServerLaunching const& s);
  void receive_loop();

  void handle_connectionless_packet(const std::string& command,
                                    const asio::ip::udp::endpoint& sender);
  void send_connectionless(const std::string& response,
                           const asio::ip::udp::endpoint& endpoint);

  void handle_getinfo(const std::string& commandline,
                      const asio::ip::udp::endpoint& sender);
  void handle_getstatus(const std::string& commandline,
                        const asio::ip::udp::endpoint& sender);
  void handle_getchallenge(const std::string& commandline,
                           const asio::ip::udp::endpoint& sender);
  void handle_connect(const std::string& commandline,
                      const asio::ip::udp::endpoint& sender);
    static asio::socket_base::message_flags handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);

  static uint32_t generate_challenge();
  static std::vector<std::string> parse_connect_args(const std::string& commandline);
  ClientInfo* find_client_by_endpoint(const asio::ip::udp::endpoint& endpoint);

  static const std::unordered_map<std::uint8_t,
    void (NetworkServer::*)(const std::string &,
      const asio::ip::udp::endpoint&)>
      _command_table;

  asio::io_context _io_c;
  std::unique_ptr<asio::ip::udp::socket> _socket;
  std::vector<std::thread> _threads;
  std::array<ClientInfo, MAX_PLAYERS> _clients;
  CircularBuffer<BUFFER_SIZE> _recv_buffer;
  uint32_t _server_id;
  bool _running = false;

  std::array<char, HOSTNAME_LENGTH> _hostname = {0};
  std::array<char, MAPNAME_LENGTH> _mapname = {0};
  int _max_players = MAX_PLAYERS;
};
