/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Server
*/

#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <semaphore>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "PackageFragmentation.hpp"
#include "ServerCommands.hpp"
#include "ServerLaunch.hpp"
#include "plugin/Byte.hpp"
#include "plugin/CircularBuffer.hpp"

class Server
{
public:
  Server(ServerLaunching const& s,
         SharedQueue<ComponentBuilder>&,
         SharedQueue<EventBuilder>&,
         std::atomic<bool>& running,
         std::counting_semaphore<> &);
  ~Server();

  void close();
  void receive_loop();

private:
  void handle_connectionless_packet(ConnectionlessCommand const& command,
                                    const asio::ip::udp::endpoint& sender);
  void handle_connected_packet(ConnectedPackage const& command,
                               const asio::ip::udp::endpoint& sender);

  void handle_connected_command(ConnectedCommand const& command,
                                const asio::ip::udp::endpoint& sender);

  void send(ByteArray const& response,
                           const asio::ip::udp::endpoint& endpoint);
  void send_connected(ByteArray const& response,
                           const asio::ip::udp::endpoint& endpoint);

  void handle_getinfo(ByteArray const& cmd,
                      const asio::ip::udp::endpoint& sender);
  void handle_getstatus(ByteArray const& cmd,
                        const asio::ip::udp::endpoint& sender);
  void handle_getchallenge(ByteArray const& cmd,
                           const asio::ip::udp::endpoint& sender);
  void handle_connect(ByteArray const& cmd,
                      const asio::ip::udp::endpoint& sender);

  void handle_event_receive(ByteArray const&, const asio::ip::udp::endpoint&);

  // static asio::socket_base::message_flags handle_receive(
  //     const asio::error_code& error, std::size_t bytes_transferred);

  void handle_package(ByteArray const&, const asio::ip::udp::endpoint&);

  static uint32_t generate_challenge();
  static std::optional<Package> parse_package(ByteArray const& package);
  static std::optional<ConnectionlessCommand> parse_connectionless_package(
      ByteArray const& package);
  static std::optional<ConnectCommand> parse_connect_command(
      ByteArray const& package);
  static std::optional<ConnectedPackage> parse_connected_package(
      ByteArray const& package);
  static std::optional<ConnectedCommand> parse_connected_command(
      ByteArray const& package);
  static std::optional<EventBuilder> parse_event_build_cmd(
      ByteArray const& package);
  static std::optional<ComponentBuilder> parse_component_build_cmd(
      ByteArray const& package);

  ClientInfo& find_client_by_endpoint(const asio::ip::udp::endpoint& endpoint);
  void remove_client_by_endpoint(const asio::ip::udp::endpoint& endpoint);

  static const std::unordered_map<
      std::uint8_t,
      void (Server::*)(ByteArray const&, const asio::ip::udp::endpoint&)>
      connectionless_table;

  static const std::unordered_map<
      std::uint8_t,
      void (Server::*)(ByteArray const&, const asio::ip::udp::endpoint&)>
      connected_table;

  std::uint32_t _current_index_sequence = 0;

  asio::io_context _io_c;
  asio::ip::udp::socket _socket;

  std::mutex _client_mutex;
  std::vector<ClientInfo> _clients;
  std::size_t _c_id_incrementator = 0;
  CircularBuffer<BUFFER_SIZE> _recv_buffer;
  std::uint32_t _server_id;

  std::string _hostname = "R-Type Server";
  std::string _mapname = "level1";
  int _max_players = MAX_PLAYERS;

  std::reference_wrapper<SharedQueue<ComponentBuilder>> _components_to_create;

  void transmit_event(EventBuilder &&to_transmit);
  std::reference_wrapper<SharedQueue<EventBuilder>> _events_to_transmit;
  std::atomic<bool>& _running;

  std::unordered_map<FragmentedPackage, ByteArray, FragmentedPackage::Hash>
      _waiting_packages;

  void send_comp();
  std::reference_wrapper<std::counting_semaphore<>> _semaphore;
  std::thread _queue_reader;
};
