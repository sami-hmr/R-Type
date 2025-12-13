/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client
*/

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <queue>
#include <semaphore>
#include <string>
#include <thread>
#include <unordered_map>

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "ClientConnection.hpp"
#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "ServerCommands.hpp"
#include "ServerLaunch.hpp"
#include "plugin/Byte.hpp"

class Client
{
public:
  Client(ClientConnection const& c,
         SharedQueue<ComponentBuilder>&,
         SharedQueue<EventBuilder>&,
         SharedQueue<EventBuilder>&,
         std::atomic<bool>& running,
         std::counting_semaphore<>&);
  ~Client();

  void close();
  void connect();

private:
  void receive_loop();
  void send(ByteArray const& command);
  void send_connected(ByteArray const& response);
  void handle_connectionless_response(ConnectionlessCommand const& response);
  void handle_connected_package(ConnectedPackage const& package);
  void handle_connected_command(ConnectedCommand const& command);
  void handle_package(ByteArray const& package);

  void handle_component_update(ByteArray const& package);
  void handle_event_creation(ByteArray const& package);

  void send_getchallenge();
  void send_connect(std::uint32_t challenge);

  void handle_challenge_response(ByteArray const& package);
  void handle_connect_response(ByteArray const& package);
  void handle_disconnect_response(ByteArray const& package);

  static std::optional<Package> parse_package(ByteArray const& package);
  static std::optional<ConnectionlessCommand> parse_connectionless_package(
      ByteArray const& package);
  static std::optional<ConnectResponse> parse_connect_response(
      ByteArray const& package);
  static std::optional<ChallengeResponse> parse_challenge_response(
      ByteArray const& package);
  static std::optional<ConnectedPackage> parse_connected_package(
      ByteArray const& package);
  static std::optional<ConnectedCommand> parse_connected_command(
      ByteArray const& package);
  static std::optional<ConnectCommand> parse_connect_command(
      ByteArray const& package);
  static std::optional<EventBuilder> parse_event_build_cmd(
      ByteArray const& package);
  static std::optional<ComponentBuilder> parse_component_build_cmd(
      ByteArray const& package);

  void transmit_component(ComponentBuilder&&);
  void transmit_event(EventBuilder&&);

  static const std::unordered_map<std::uint8_t,
                                  void (Client::*)(ByteArray const&)>
      connectionless_table;

  static const std::unordered_map<std::uint8_t,
                                  void (Client::*)(ByteArray const&)>
      connected_table;

  asio::io_context _io_c;
  asio::ip::udp::socket _socket;
  asio::ip::udp::endpoint _server_endpoint;

  ConnectionState _state = ConnectionState::DISCONNECTED;
  std::uint8_t _client_id = 0;
  std::uint32_t _server_id = 0;
  std::string _player_name = "Player";

  std::reference_wrapper<SharedQueue<ComponentBuilder>> _components_to_create;
  std::reference_wrapper<SharedQueue<EventBuilder>> _events_to_transmit;
  std::reference_wrapper<SharedQueue<EventBuilder>> _event_to_exec;
  std::reference_wrapper<std::atomic<bool>> _running;

  std::unordered_map<std::uint32_t, ByteArray> _waiting_packages;

  void send_evt();
  std::reference_wrapper<std::counting_semaphore<>> _semaphore;
  std::thread _queue_reader;

  std::uint32_t _current_index_sequence = 0;
};
