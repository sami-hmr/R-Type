/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Server
*/

#pragma once

#include <queue>
#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <unordered_map>

#include <asio/ip/udp.hpp>
#include <asio/error_code.hpp>
#include <asio/io_context.hpp>

#include "NetworkShared.hpp"
#include "plugin/CircularBuffer.hpp"
#include "ServerCommands.hpp"
#include "NetworkCommun.hpp"
#include "ServerLaunch.hpp"
#include "plugin/Byte.hpp"

class Server
{
  public:
    Server(ServerLaunching const& s, SharedQueue<ComponentBuilder> &, std::atomic<bool> &running);
    ~Server();

    void close();
    void receive_loop();

  private:
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
    // static asio::socket_base::message_flags handle_receive(
    //     const asio::error_code& error, std::size_t bytes_transferred);

    void handle_package(ByteArray const&, const asio::ip::udp::endpoint&);

    static uint32_t generate_challenge();
    std::optional<Package> parse_package(ByteArray const& package);
    std::optional<ConnectionlessCommand> parse_connectionless_package(
        ByteArray const& package);
    std::optional<ConnectCommand> parse_connect_command(ByteArray const& cmd);

    ClientInfo& find_client_by_endpoint(const asio::ip::udp::endpoint& endpoint);

    static const std::unordered_map<std::uint8_t,
      void (Server::*)(ByteArray const&, const asio::ip::udp::endpoint&)>
      command_table;

    asio::io_context _io_c;
    asio::ip::udp::socket _socket;
    std::vector<ClientInfo> _clients;
    CircularBuffer<BUFFER_SIZE> _recv_buffer;
    std::uint32_t _server_id;

    std::string _hostname = "R-Type Server";
    std::string _mapname = "level1";
    int _max_players = MAX_PLAYERS;

    std::reference_wrapper<SharedQueue<ComponentBuilder>> _shared_queue;
    std::atomic<bool> &_running;
};
