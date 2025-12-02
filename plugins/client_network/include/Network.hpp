#pragma once

#include <cstdint>
#include <functional>
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

#define MAGIC_SEQUENCE 0x67676767

enum class ConnectionState
{
  DISCONNECTED,
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

  void handle_challenge_response(const std::vector<std::string>& args);
  void handle_connect_response(const std::vector<std::string>& args);
  void handle_disconnect_response(const std::vector<std::string>& args);

  std::vector<std::string> parse_args(const std::string& response);

  static const std::unordered_map<std::string,
                                  void (NetworkClient::*)(
                                      const std::vector<std::string>&)>
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
