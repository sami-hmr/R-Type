#include <algorithm>
#include <cstdint>
#include <random>

#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "network/server/Server.hpp"

const std::unordered_map<std::uint8_t,
                         void (Server::*)(ByteArray const&,
                                          const asio::ip::udp::endpoint&)>
    Server::connectionless_table = {
        {GETINFO, &Server::handle_getinfo},
        {GETSTATUS, &Server::handle_getstatus},
        {GETCHALLENGE, &Server::handle_getchallenge},
        {CONNECT, &Server::handle_connect},
};

void Server::handle_connectionless_packet(ConnectionlessCommand const& command,
                                          const asio::ip::udp::endpoint& sender)
{
  NETWORK_LOGGER("server",
                 std::uint8_t(LogLevel::DEBUG),
                 std::format("Received connectionless packet: '{}'",
                             command.command_code));

  try {
    (this->*(connectionless_table.find(command.command_code)->second))(
        command.command, sender);
  } catch (std::out_of_range const&) {
    NETWORK_LOGGER("server",
                   std::uint8_t(LogLevel::WARNING),
                   std::format("Unknown command: {}", command.command_code));
  }
}

void Server::handle_getinfo(ByteArray const& cmd,
                            const asio::ip::udp::endpoint& sender)
{
  if (!cmd.empty()) {
    NETWORK_LOGGER("server",
                   std::uint8_t(LogLevel::WARNING),
                   "Invalid getinfo command: command not empty");
    return;
  }
  ByteArray pkg = type_to_byte<Byte>(INFORESPONSE)
      + ByteArray(_hostname.begin(), _hostname.end())
      + ByteArray(_mapname.begin(), _mapname.end()) + type_to_byte(COOP)
      + type_to_byte(_max_players)
      + type_to_byte<Byte>(CURRENT_PROTOCOL_VERSION);

  send(pkg, sender);
}

void Server::handle_getstatus(ByteArray const& cmd,
                              const asio::ip::udp::endpoint& sender)
{
  if (!cmd.empty()) {
    NETWORK_LOGGER("server",
                   std::uint8_t(LogLevel::WARNING),
                   "Invalid getstatus command: command not empty");
    return;
  }

  ByteArray pkg = type_to_byte<Byte>(INFORESPONSE)
      + ByteArray(_hostname.begin(), _hostname.end())
      + ByteArray(_mapname.begin(), _mapname.end()) + type_to_byte(COOP)
      + type_to_byte(_max_players)
      + type_to_byte<Byte>(CURRENT_PROTOCOL_VERSION);

  for (const auto& client : _clients) {
    if (client.state == ClientState::CONNECTED) {
      pkg = pkg + type_to_byte<std::uint32_t>(client.score)
          + type_to_byte<std::uint8_t>(client.ping)
          + ByteArray(client.player_name.begin(), client.player_name.end());
    }
  }

  send(pkg, sender);
}

void Server::handle_getchallenge(ByteArray const& cmd,
                                 const asio::ip::udp::endpoint& sender)
{
  if (!cmd.empty()) {
    NETWORK_LOGGER("server",
                   std::uint8_t(LogLevel::WARNING),
                   "Invalid getchallenge command: command not empty");
    return;
  }
  uint32_t challenge = generate_challenge();

  ClientInfo client;

  client.endpoint = sender;
  client.challenge = challenge;
  client.state = ClientState::CHALLENGING;

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
      NETWORK_LOGGER(
          "server", std::uint8_t(LogLevel::WARNING), "Invalid challenge");
      this->_client_mutex.unlock();
      return;
    }
    uint8_t client_id = this->_c_id_incrementator;
    this->_c_id_incrementator++;

    client.client_id = client_id;
    client.player_name = parsed->player_name;
    client.state = ClientState::CONNECTED;
    this->_client_mutex.unlock();

    NETWORK_LOGGER("server",
                   std::uint8_t(LogLevel::INFO),
                   std::format("Player '{}' connected as client {}",
                               parsed->player_name,
                               static_cast<int>(client_id)));

    ByteArray pkg = type_to_byte<Byte>(CONNECTRESPONSE)
        + type_to_byte<std::uint8_t>(client_id)
        + type_to_byte<std::uint32_t>(_server_id);

    send(pkg, sender);
    this->transmit_event_to_server(
        EventBuilder("NewConnection", NewConnection(client_id).to_bytes()));
  } catch (ClientNotFound&) {
    NETWORK_LOGGER(
        "server", std::uint8_t(LogLevel::WARNING), "Invalid challenge");
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

ClientInfo& Server::find_client_by_endpoint(
    const asio::ip::udp::endpoint& endpoint)
{
  for (auto& client : _clients) {
    if (client.state != ClientState::DISCONNECTED
        && client.endpoint == endpoint)
    {
      return client;
    }
  }
  throw ClientNotFound("client not found");
}

ClientInfo& Server::find_client_by_id(std::size_t id)
{
  for (auto& client : _clients) {
    if (client.state != ClientState::DISCONNECTED && client.client_id == id) {
      return client;
    }
  }
  throw ClientNotFound("client not found");
}

void Server::remove_client_by_endpoint(const asio::ip::udp::endpoint& endpoint)
{
  this->_client_mutex.lock();
  auto it = std::find_if(this->_clients.begin(),
                         this->_clients.end(),
                         [endpoint](ClientInfo const& c)
                         { return c.endpoint == endpoint; });
  if (it != this->_clients.end()) {
    this->_clients.erase(it);
  }
  this->_client_mutex.unlock();
}
