/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Server
*/
#include "Server.hpp"
#include "Network.hpp"
// #include "plugin/events/Events.hpp"

const std::unordered_map<std::uint8_t,
                         void (Server::*)(
                             ByteArray const&, const asio::ip::udp::endpoint&)>
    Server::_command_table = {
        {GETINFO, &Server::handle_getinfo},
        {GETSTATUS, &Server::handle_getstatus},
        {GETCHALLENGE, &Server::handle_getchallenge},
        {CONNECT, &Server::handle_connect},
};

Server::Server(ServerLaunching const& s, std::queue<std::shared_ptr<ByteArray>> &cmpnts, bool running) : _socket(_io_c, asio::ip::udp::endpoint(asio::ip::udp::v4(), s.port)), _running(running), _components_to_create(std::reference_wrapper(cmpnts))
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis;
    _server_id = dis(gen);
}

void Server::close()
{
  if (_socket.is_open()) {
    _socket.close();
  }
}

Server::~Server()
{
  _socket.close();
}

void Server::receive_loop()
{
  CircularBuffer<BUFFER_SIZE> recv_buf;
  asio::ip::udp::endpoint sender_endpoint;

  while (_running) {
    try {
      std::error_code ec;
      std::size_t len =
          recv_buf.read_socket(this->_socket, sender_endpoint, ec);
      if (len > 0) {
        NETWORK_LOGGER("server",
               std::uint8_t(LogLevel::DEBUG),
               std::format("received buffer, size : {}", len));
      }

      if (ec) {
        if (_running) {
          NETWORK_LOGGER("server",
                 std::uint8_t(LogLevel::ERROR),
                 std::format("Receive error: {}", ec.message()));
        }
        break;
      }

      while (std::optional<ByteArray> p = recv_buf.extract(PROTOCOL_EOF)) {
        NETWORK_LOGGER("server", std::uint8_t(LogLevel::DEBUG), "package extracted");
        // for (auto it : *p) {
        //   std::cout << (int)it << std::endl;
        // }
        this->handle_package(*p, sender_endpoint);
      }
    } catch (std::exception& e) {
      if (_running) {
        NETWORK_LOGGER("server",
               std::uint8_t(LogLevel::ERROR),
               std::format("Error in receive loop: {}", e.what()));
      }
      break;
    }
  }

  NETWORK_LOGGER("server", std::uint8_t(LogLevel::INFO), "Server receive loop ended");
}

void Server::handle_package(ByteArray const& package,
                                   const asio::ip::udp::endpoint& sender)
{
  std::optional<Package> pkg = this->parse_package(package);

  if (!pkg) {
    return;
  }
  if (pkg->magic != MAGIC_SEQUENCE) {
    NETWORK_LOGGER("server", std::uint8_t(LogLevel::DEBUG), "Invalid magic sequence, ignoring.");
    return;
  }
  auto const& parsed = this->parse_connectionless_package(pkg->real_package);
  // TODO: handle in different function connected and not connected package
  if (!parsed) {
    return;
  }
  handle_connectionless_packet(parsed.value(), sender);
}

void Server::handle_connectionless_packet(
    ConnectionlessCommand const& command, const asio::ip::udp::endpoint& sender)
{
  NETWORK_LOGGER("server",
         std::uint8_t(LogLevel::DEBUG),
         std::format("Received connectionless packet: '{}'",
                     command.command_code));

  try {
    (this->*(_command_table.find(command.command_code)->second))(
        command.command, sender);
  } catch (std::out_of_range const&) {
    NETWORK_LOGGER("server",
           std::uint8_t(LogLevel::WARNING),
           std::format("Unknown command: {}", command.command_code));
  }
}

void Server::send_connectionless(ByteArray const& response,
                                        const asio::ip::udp::endpoint& endpoint)
{
  ByteArray pkg = MAGIC_SEQUENCE + response + PROTOCOL_EOF;

  _socket.send_to(asio::buffer(pkg), endpoint);

  NETWORK_LOGGER("server",
         std::uint8_t(LogLevel::DEBUG),
         std::format("Sent connectionless package of size: {}", pkg.size()));
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

  send_connectionless(pkg, sender);
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

  send_connectionless(pkg, sender);
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

  _clients.push_back(client);

  ByteArray pkg = type_to_byte<Byte>(CHALLENGERESPONSE)
      + type_to_byte<std::uint32_t>(challenge);
  send_connectionless(pkg, sender);
}

void Server::handle_connect(ByteArray const& cmd,
                                   const asio::ip::udp::endpoint& sender)
{
  std::optional<ConnectCommand> parsed = this->parse_connect_command(cmd);
  if (!parsed) {
    return;
  }

  try {
    ClientInfo& client = find_client_by_endpoint(sender);

    if (client.state != ClientState::CHALLENGING
        || client.challenge != parsed->challenge)
    {
      NETWORK_LOGGER("server", std::uint8_t(LogLevel::WARNING), "Invalid challenge");
      return;
    }
    uint8_t client_id = _clients.size(); //TODO: change to incrementator uint32 in the wrapper class

    client.client_id = client_id;
    client.player_name = parsed->player_name;
    client.state = ClientState::CONNECTED;

    NETWORK_LOGGER("server",
           std::uint8_t(LogLevel::INFO),
           std::format("Player '{}' connected as client {}",
                       parsed->player_name,
                       static_cast<int>(client_id)));

    ByteArray pkg = type_to_byte<Byte>(CONNECTRESPONSE)
        + type_to_byte<std::uint8_t>(client_id)
        + type_to_byte<std::uint32_t>(_server_id);

    send_connectionless(pkg, sender);

  } catch (ClientNotFound&) {
    NETWORK_LOGGER("server", std::uint8_t(LogLevel::WARNING), "Invalid challenge");
    return;
  }
}

uint32_t Server::generate_challenge()
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
  throw ClientNotFound("");
}
