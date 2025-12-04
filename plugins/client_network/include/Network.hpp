#pragma once

#include <cstdint>
// #include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "ClientConnection.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

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

#define DISCONNECT_RESP_CMD_SIZE 4
#define CHALLENGE_RESP_CMD_SIZE 4
#define CONNECT_RESP_CMD_SIZE 5

#define END_OF_CMD std::uint8_t(0x00)


// indexes

#define CMD_INDEX 0

#define PROTOCOL_INDEX 0

//- Challend resp -//

#define CHALLENGE_CLG_INDEX 1

//- Connect command -//

#define ERR_MESS_DSCNT_INDEX 1

//- Connect command -//

#define CHALLENGE_CNT_INDEX 1
#define PLAYERNAME_CNT_INDEX 2

//- Connect response -//

#define CLIENT_ID_CNT_RESP_INDEX 1
#define SERVER_ID_CNT_RESP_INDEX 2


// size in byte

#define CONNECT_COMMAND_SIZE 37
#define PLAYERNAME_MAX_SIZE 32
#define ERROR_MSG_SIZE 32
#define CHALLENGE_SIZE 4
#define SERVER_ID_SIZE 4
#define CLIENT_ID_SIZE 1
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

enum class ConnectionState : std::uint8_t
{
  DISCONNECTED = 0,
  CHALLENGING,
  CONNECTING,
  CONNECTED
};

class NetworkClient : public APlugin
{
public:
  NetworkClient(Registery& r, EntityLoader& l);
  ~NetworkClient() override;

private:
  void connection_thread(ClientConnection const& c);
  void receive_loop();

  void send_connectionless(const std::string& command);
  void handle_connectionless_response(const std::string& response);

  void send_getchallenge();
  void send_connect(uint32_t challenge, const std::string& player_name);

  void handle_challenge_response(const std::string& commandline);
  void handle_connect_response(const std::string& commandline);
  void handle_disconnect_response(const std::string& commandline);

  static std::vector<std::string> parse_challenge_response(const std::string& resp);
  static std::vector<std::string> parse_connect_response(const std::string& resp);
  static std::vector<std::string> parse_disconnect_response(const std::string& resp);

  bool is_valid_response_format(const std::vector<std::string> &args,
    std::uint8_t requested_size, const std::string &err_mess);


  static const std::unordered_map<std::uint8_t,
    void (NetworkClient::*)(const std::string &)>
      _command_table;

  asio::io_context _io_c;
  std::unique_ptr<asio::ip::udp::socket> _socket;
  asio::ip::udp::endpoint _server_endpoint;
  std::vector<std::thread> _threads;

  ConnectionState _state = ConnectionState::DISCONNECTED;
  uint8_t _client_id = 0;
  uint32_t _server_id = 0;
  std::string _player_name;
  bool _running = false;
};
