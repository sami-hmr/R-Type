#include <algorithm>
#include <chrono>
#include <cstdint>
#include <random>

#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "network/server/Server.hpp"

const std::unordered_map<std::uint8_t,
                         void (Server::*)(ByteArray const&,
                                          const asio::ip::udp::endpoint&)>
    Server::connectionless_table = {
        {GETCHALLENGE, &Server::handle_getchallenge},
        {CONNECT, &Server::handle_connect},
};

void Server::handle_connectionless_packet(ConnectionlessCommand const& command,
                                          const asio::ip::udp::endpoint& sender)
{
  LOGGER_EVTLESS(LogLevel::DEBUG,
              "server",
              std::format("Received connectionless packet: '{}'",
                          command.command_code));

  try {
    (this->*(connectionless_table.at(command.command_code)))(command.command,
                                                             sender);
  } catch (std::out_of_range const&) {
    LOGGER_EVTLESS(LogLevel::WARNING,
                "server",
                std::format("Unknown command: {}", command.command_code));
  }
}

void Server::handle_getchallenge(ByteArray const& cmd,
                                 const asio::ip::udp::endpoint& sender)
{
  if (!cmd.empty()) {
    LOGGER_EVTLESS(LogLevel::WARNING,
                "server",
                "Invalid getchallenge command: command not empty");
    return;
  }
  uint32_t challenge = generate_challenge();

  ClientInfo client;

  client.endpoint = sender;
  client.challenge = challenge;
  client.state = ClientState::CHALLENGING;
  client.last_ping =
      std::chrono::steady_clock::now().time_since_epoch().count();

  this->_client_mutex.lock();
  _clients.push_back(client);
  this->_client_mutex.unlock();

  ByteArray pkg = type_to_byte<Byte>(CHALLENGERESPONSE)
      + type_to_byte<std::uint32_t>(challenge);
  send(pkg, sender);
}

void Server::handle_connect(ByteArray const& cmd,
                            const asio::ip::udp::endpoint& sender)
{
  std::optional<ConnectCommand> parsed = parse_connect_command(cmd);
  if (!parsed) {
    return;
  }

  try {
    this->_client_mutex.lock();
    ClientInfo& client = find_client_by_endpoint(sender);

    if (client.state != ClientState::CHALLENGING
        || client.challenge != parsed->challenge)
    {
      LOGGER_EVTLESS(LogLevel::WARNING, "server", "Invalid challenge");
      this->_client_mutex.unlock();
      return;
    }
    uint8_t client_id = this->_c_id_incrementator;
    this->_c_id_incrementator++;

    client.client_id = client_id;
    client.player_name = parsed->player_name;
    client.state = ClientState::CONNECTED;
    this->_client_mutex.unlock();

    LOGGER_EVTLESS(LogLevel::INFO,
                "server",
                std::format("Player '{}' connected as client {}",
                            parsed->player_name,
                            static_cast<int>(client_id)));

    ByteArray pkg = type_to_byte<Byte>(CONNECTRESPONSE)
        + type_to_byte<std::uint8_t>(client_id)
        + type_to_byte<std::uint32_t>(_server_id);

    send(pkg, sender);
    this->transmit_event_to_server(
        EventBuilder("NewConnection", NewConnection(client_id).to_bytes()));
  } catch (ClientNotFound const& e) {
    LOGGER_EVTLESS(
        LogLevel::WARNING,
        "server",
        std::format("Invalid challenge during connect: {} (context: {})",
                    e.what(),
                    e.format_context()));
    return;
  }
}

std::uint32_t Server::generate_challenge()
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<uint32_t> dis(1, UINT32_MAX);
  return dis(gen);
}
