/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client
*/

#pragma once

#include <atomic>
#include <functional>
#include <queue>
#include <string>
#include <cstdint>
#include <unordered_map>

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "ClientConnection.hpp"
#include "NetworkShared.hpp"
#include "ServerCommands.hpp"
#include "NetworkCommun.hpp"
#include "ServerLaunch.hpp"
#include "plugin/Byte.hpp"

class Client
{
  public:
    Client(ClientConnection const& c, SharedQueue<ComponentBuilder> &, SharedQueue<EventBuilder> &, std::atomic<bool> &running);
    ~Client();

    void close();
    void connect();

  private:

    void receive_loop();
    void send_connectionless(ByteArray const& command);
    void handle_connectionless_response(ByteArray const& response);
    void handle_package(ByteArray const& package);

    void send_getchallenge();
    void send_connect(std::uint32_t challenge);

    void handle_challenge_response(ByteArray const& package);
    void handle_connect_response(ByteArray const& package);
    void handle_disconnect_response(ByteArray const& package);

    std::optional<Package> parse_package(ByteArray const& package);
    std::optional<ConnectionlessCommand> parse_connectionless_package(
        ByteArray const& package);
    std::optional<ConnectResponse> parse_connect_response(ByteArray const& package);
    std::optional<ChallengeResponse> parse_challenge_response(ByteArray const& package);

    void transmit_event(EventBuilder &&);

    static const std::unordered_map<std::uint8_t,
                                    void (Client::*)(ByteArray const&)>
        _command_table;

    asio::io_context _io_c;
    asio::ip::udp::socket _socket;
    asio::ip::udp::endpoint _server_endpoint;

    ConnectionState _state = ConnectionState::DISCONNECTED;
    std::uint8_t _client_id = 0;
    std::uint32_t _server_id = 0;
    std::string _player_name = "Player";

    std::reference_wrapper<SharedQueue<ComponentBuilder>> _components_to_create;
    std::reference_wrapper<SharedQueue<EventBuilder>> _events_to_transmit;
    std::reference_wrapper<std::atomic<bool>> _running;
};
