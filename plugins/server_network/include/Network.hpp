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

#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

// #include <boost/system/error_code.hpp>

#include "CustomException.hpp"
#include "NetworkCommun.hpp"
#include "ServerCommands.hpp"
#include "ServerLaunch.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Byte.hpp"
#include "plugin/CircularBuffer.hpp"
#include "plugin/EntityLoader.hpp"

class NetworkServer : public APlugin
{
public:
  NetworkServer(Registery& r, EntityLoader& l);
  ~NetworkServer() override;

private:
  void launch_server(ServerLaunching const& s);
  void receive_loop();

  void handle_connectionless_packet(ConnectionlessCommand const& command,
                                    const asio::ip::udp::endpoint& sender);
  void send_connectionless(ByteArray const& response,
                           const asio::ip::udp::endpoint& endpoint);

  void handle_getinfo(ByteArray const& cmd,
                      const asio::ip::udp::endpoint& sender);
  void handle_getstatus(ByteArray const& cmd,
                        const asio::ip::udp::endpoint& sender);
  void handle_getchallenge(ByteArray const& cmd,
                           const asio::ip::udp::endpoint& sender);
  void handle_connect(ByteArray const& cmd,
                      const asio::ip::udp::endpoint& sender);
  static asio::socket_base::message_flags handle_receive(
      const asio::error_code& error, std::size_t bytes_transferred);

  void handle_package(ByteArray const&, const asio::ip::udp::endpoint&);

  static uint32_t generate_challenge();
  std::optional<Package> parse_package(ByteArray const& package);
  std::optional<ConnectionlessCommand> parse_connectionless_package(
      ByteArray const& package);
  std::optional<ConnectCommand> parse_connect_command(ByteArray const& cmd);

  ClientInfo& find_client_by_endpoint(const asio::ip::udp::endpoint& endpoint);

  static const std::unordered_map<
      std::uint8_t,
      void (NetworkServer::*)(ByteArray const&, const asio::ip::udp::endpoint&)>
      _command_table;

  asio::io_context _io_c;
  std::unique_ptr<asio::ip::udp::socket> _socket;
  std::vector<std::thread> _threads;
  std::vector<ClientInfo> _clients;
  CircularBuffer<BUFFER_SIZE> _recv_buffer;
  uint32_t _server_id;
  bool _running = false;

  std::array<char, HOSTNAME_LENGTH> _hostname = {0};
  std::array<char, MAPNAME_LENGTH> _mapname = {0};
  int _max_players = MAX_PLAYERS;
};

CUSTOM_EXCEPTION(ClientNotFound)
