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
         SharedQueue<ComponentBuilderId>& comp_queue,
         SharedQueue<EventBuilderId>& event_to_client,
         SharedQueue<EventBuilder>& event_to_server,
         std::atomic<bool>& running);
  ~Server();

  void close();
  void receive_loop();

  void disconnect_client(std::size_t client_id);
  std::vector<std::size_t> watch_disconected_clients();

private:
  void handle_connectionless_packet(ConnectionlessCommand const& command,
                                    const asio::ip::udp::endpoint& sender);
  void handle_connected_packet(ConnectedPackage const& command,
                               const asio::ip::udp::endpoint& sender);

  void handle_connected_command(ConnectedCommand const& command,
                                const asio::ip::udp::endpoint& sender);

  void send(ByteArray const& response, const asio::ip::udp::endpoint& endpoint,  bool hearthbeat = false);
  void send_connected(ByteArray const& response, ClientInfo& client);
  void handle_getchallenge(ByteArray const& cmd,
                           const asio::ip::udp::endpoint& sender);
  void handle_connect(ByteArray const& cmd,
                      const asio::ip::udp::endpoint& sender);

  void handle_event_receive(ByteArray const&, const asio::ip::udp::endpoint&);
  void handle_hearthbeat(ByteArray const &, const asio::ip::udp::endpoint&);

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
  static std::optional<HearthBeat> parse_hearthbeat_cmd(
      ByteArray const& package);
  ClientInfo& find_client_by_endpoint(const asio::ip::udp::endpoint& endpoint);
  ClientInfo& find_client_by_id(std::size_t id);
  void remove_client_by_endpoint(const asio::ip::udp::endpoint& endpoint);
  void remove_client_by_id(std::size_t client_id);

  static const std::unordered_map<
      std::uint8_t,
      void (Server::*)(ByteArray const&, const asio::ip::udp::endpoint&)>
      connectionless_table;

  static const std::unordered_map<
      std::uint8_t,
      void (Server::*)(ByteArray const&, const asio::ip::udp::endpoint&)>
      connected_table;


  asio::io_context _io_c;
  asio::ip::udp::socket _socket;

  std::mutex _client_mutex;
  static const std::size_t client_disconect_timout = 5000000000; // 5 seconds
  std::vector<ClientInfo> _clients;
  std::size_t _c_id_incrementator = 0;
  CircularBuffer<BUFFER_SIZE> _recv_buffer;
  std::uint32_t _server_id;

  std::reference_wrapper<SharedQueue<ComponentBuilderId>> _components_to_create;

  void transmit_event_to_client(EventBuilderId const& to_transmit);
  void send_event_to_client();
  std::reference_wrapper<SharedQueue<EventBuilderId>> _events_queue_to_client;

  void transmit_event_to_server(EventBuilder const& to_transmit);
  std::reference_wrapper<SharedQueue<EventBuilder>> _events_queue_to_serv;

  std::atomic<bool>& _running;

  std::unordered_map<FragmentedPackage, ByteArray, FragmentedPackage::Hash>
      _waiting_packages;

  void send_comp();
  std::vector<std::thread> _queue_readers;
};

CUSTOM_EXCEPTION(ClientNotFound)
