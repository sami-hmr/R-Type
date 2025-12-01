#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "Events.hpp"
#include "ServerLaunch.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/CircularBuffer.hpp"
#include "plugin/EntityLoader.hpp"

#define MAX_PLAYERS 4
#define MAGIC_SEQUENCE 0x67676767

enum class ClientState
{
  DISCONNECTED,
  CHALLENGING,
  CONNECTED
};

struct ClientInfo
{
  asio::ip::udp::endpoint endpoint;
  ClientState state = ClientState::DISCONNECTED;
  uint32_t challenge = 0;
  uint8_t client_id = 0;
  std::string player_name;
  uint32_t last_sequence = 0;
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

  void handle_getinfo(const std::vector<std::string>& args,
                      const asio::ip::udp::endpoint& sender);
  void handle_getstatus(const std::vector<std::string>& args,
                        const asio::ip::udp::endpoint& sender);
  void handle_getchallenge(const std::vector<std::string>& args,
                           const asio::ip::udp::endpoint& sender);
  void handle_connect(const std::vector<std::string>& args,
                      const asio::ip::udp::endpoint& sender);

  uint32_t generate_challenge();
  std::vector<std::string> parse_args(const std::string& command);
  ClientInfo* find_client_by_endpoint(const asio::ip::udp::endpoint& endpoint);

  static const std::unordered_map<std::string,
                                  void (NetworkServer::*)(
                                      const std::vector<std::string>&,
                                      const asio::ip::udp::endpoint&)>
      _command_table;

  asio::io_context _io_c;
  std::unique_ptr<asio::ip::udp::socket> _socket;
  std::vector<std::thread> _threads;
  std::array<ClientInfo, MAX_PLAYERS> _clients;
  CircularBuffer<2048> _recv_buffer;
  uint32_t _server_id;
  bool _running = false;

  std::string _hostname = "R-Type Server";
  std::string _mapname = "level1";
  int _max_players = MAX_PLAYERS;
};
