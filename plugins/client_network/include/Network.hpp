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
#include "NetworkCommun.hpp"
#include "ServerCommands.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Byte.hpp"
#include "plugin/CircularBuffer.hpp"
#include "plugin/EntityLoader.hpp"

class NetworkClient : public APlugin
{
public:
  NetworkClient(Registery& r, EntityLoader& l);
  ~NetworkClient() override;

private:
  void connection_thread(ClientConnection const& c);
  void receive_loop();

  void send_connectionless(ByteArray const& command);
  void handle_connectionless_response(ByteArray const& response);
  void handle_package(ByteArray const& package);

  void send_getchallenge();
  void send_connect(uint32_t challenge, ByteArray const& player_name);

  void handle_challenge_response(ByteArray const& commandline);
  void handle_connect_response(ByteArray const& commandline);
  void handle_disconnect_response(ByteArray const& commandline);

  std::optional<Package> parse_package(ByteArray const& package);

  static const std::unordered_map<std::uint8_t,
                                  void (NetworkClient::*)(ByteArray const&)>
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
