#include <array>
#include <cstddef>
#include <cstring>
#include <format>
#include <functional>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <thread>

#include "Network.hpp"

#include "NetworkCommun.hpp"
#include "ParserTypes.hpp"
#include "ServerCommands.hpp"
#include "ecs/Registery.hpp"
#include "plugin/Byte.hpp"
#include "plugin/CircularBuffer.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Events.hpp"

const std::unordered_map<std::uint8_t,
                         void (NetworkServer::*)(
                             ByteArray const&, const asio::ip::udp::endpoint&)>
    NetworkServer::_command_table = {
        {GETINFO, &NetworkServer::handle_getinfo},
        {GETSTATUS, &NetworkServer::handle_getstatus},
        {GETCHALLENGE, &NetworkServer::handle_getchallenge},
        {CONNECT, &NetworkServer::handle_connect},
};

NetworkServer::NetworkServer(Registery& r, EntityLoader& l)
    : APlugin(r, l, {}, {})
    , _hostname("R-Type Server")
    , _mapname("level1")
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> dis;
  _server_id = dis(gen);

  this->_registery.get().on<ServerLaunching>(
      [this](ServerLaunching const& s)
      {
        this->_threads.emplace_back([this, s]() { this->launch_server(s); });
      });

  this->_registery.get().on<ShutdownEvent>(
      [this](ShutdownEvent const& event)
      {
        _running = false;
        if (_socket) {
          LOGGER("server",
                 LogLevel::INFO,
                 std::format("Shutdown requested: {}", event.reason));
          _socket->close();
        }
      });

  this->_registery.get().on<CleanupEvent>(
      [this](CleanupEvent const&)
      {
        _running = false;
        if (_socket) {
          LOGGER("server", LogLevel::DEBUG, "Cleanup requested");
          _socket->close();
        }
      });
}

NetworkServer::~NetworkServer()
{
  _running = false;
  if (_socket) {
    _socket->close();
  }
  for (auto& t : this->_threads) {
    t.join();
  }
}

void NetworkServer::launch_server(ServerLaunching const& s)
{
  try {
    _socket = std::make_unique<asio::ip::udp::socket>(
        _io_c, asio::ip::udp::endpoint(asio::ip::udp::v4(), s.port));

    _running = true;

    LOGGER("server",
           LogLevel::INFO,
           std::format("Server started on port {}", s.port));

    receive_loop();
  } catch (std::exception& e) {
    LOGGER("server",
           LogLevel::ERROR,
           std::format("Failed to start server: {}", e.what()));
  }
}

asio::socket_base::message_flags NetworkServer::handle_receive(
    const asio::error_code& UNUSED error, std::size_t UNUSED bytes_transferred)
{
  asio::socket_base::message_flags flag = MSG_OOB;
  std::cout << "handled the reception" << std::endl;
  return flag;
}

void NetworkServer::receive_loop()
{
  CircularBuffer<BUFFER_SIZE> recv_buf;
  asio::ip::udp::endpoint sender_endpoint;

  while (_running) {
    try {
      std::error_code ec;
      std::size_t len =
          recv_buf.read_socket(*this->_socket, sender_endpoint, ec);
      if (len > 0) {
        LOGGER("server",
               LogLevel::DEBUG,
               std::format("received buffer, size : {}", len));
      }

      if (ec) {
        if (_running) {
          LOGGER("server",
                 LogLevel::ERROR,
                 std::format("Receive error: {}", ec.message()));
        }
        break;
      }

      while (std::optional<ByteArray> p = recv_buf.extract(PROTOCOL_EOF)) {
        LOGGER("server", LogLevel::DEBUG, "package extracted");
        // for (auto it : *p) {
        //   std::cout << (int)it << std::endl;
        // }
        this->handle_package(*p, sender_endpoint);
      }
    } catch (std::exception& e) {
      if (_running) {
        LOGGER("server",
               LogLevel::ERROR,
               std::format("Error in receive loop: {}", e.what()));
      }
      break;
    }
  }

  LOGGER("server", LogLevel::INFO, "Server receive loop ended");
}

void NetworkServer::handle_package(ByteArray const& package,
                                   const asio::ip::udp::endpoint& sender)
{
  std::optional<Package> pkg = this->parse_package(package);

  if (!pkg) {
    return;
  }
  if (pkg->magic != MAGIC_SEQUENCE) {
    LOGGER("server", LogLevel::DEBUG, "Invalid magic sequence, ignoring.");
    return;
  }
  auto const& parsed = this->parse_connectionless_package(pkg->real_package);
  // TODO: handle in different function connected and not connected package
  if (!parsed) {
    return;
  }
  handle_connectionless_packet(parsed.value(), sender);
}

void NetworkServer::handle_connectionless_packet(
    ConnectionlessCommand const& command, const asio::ip::udp::endpoint& sender)
{
  LOGGER("server",
         LogLevel::DEBUG,
         std::format("Received connectionless packet: '{}'",
                     command.command_code));

  try {
    (this->*(_command_table.find(command.command_code)->second))(
        command.command, sender);
  } catch (std::out_of_range const&) {
    LOGGER("server",
           LogLevel::WARNING,
           std::format("Unknown command: {}", command.command_code));
  }
}

void NetworkServer::send_connectionless(ByteArray const& response,
                                        const asio::ip::udp::endpoint& endpoint)
{
    ByteArray pkg = type_to_byte(MAGIC_SEQUENCE) + response + type_to_byte(PROTOCOL_EOF_NUMBER);

  _socket->send_to(asio::buffer(pkg), endpoint);

  LOGGER("server",
         LogLevel::DEBUG,
         std::format("Sent: '{}'", static_cast<int>(response[CMD_INDEX])));
}

void NetworkServer::handle_getinfo(ByteArray const& cmd,
                                   const asio::ip::udp::endpoint& sender)
{
  if (!cmd.empty()) {
    LOGGER("server",
           LogLevel::WARNING,
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

void NetworkServer::handle_getstatus(ByteArray const& cmd,
                                     const asio::ip::udp::endpoint& sender)
{
  if (!cmd.empty()) {
    LOGGER("server",
           LogLevel::WARNING,
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

void NetworkServer::handle_getchallenge(ByteArray const& cmd,
                                        const asio::ip::udp::endpoint& sender)
{
  if (!cmd.empty()) {
    LOGGER("server",
           LogLevel::WARNING,
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

void NetworkServer::handle_connect(ByteArray const& cmd,
                                   const asio::ip::udp::endpoint& sender)
{
  std::optional<ConnectCommand> parsed = this->parse_connect_command(cmd);
  if (!parsed) {
    return;
  }

  if (parsed->protocol != CURRENT_PROTOCOL_VERSION) {
    std::array<char, ERROR_MSG_SIZE> msg {};
    std::string error_message = "Protocol version mismatch";
    std::copy(error_message.data(),
              error_message.data() + error_message.length(),
              msg.begin());
    ByteArray pkg =
        type_to_byte<Byte>(DISCONNECT) + ByteArray(msg.begin(), msg.end());
    send_connectionless(pkg, sender);
    return;
  }

  try {
    ClientInfo& client = find_client_by_endpoint(sender);

    if (client.state != ClientState::CHALLENGING
        || client.challenge != parsed->challenge)
    {
      LOGGER("server", LogLevel::WARNING, "Invalid challenge");
      return;
    }

    uint8_t client_id = _clients.size();

    client.client_id = client_id;
    std::copy(parsed->player_name.data(),
              parsed->player_name.data() + parsed->player_name.length(),
              client.player_name.begin());
    client.state = ClientState::CONNECTED;

    LOGGER("server",
           LogLevel::INFO,
           std::format("Player '{}' connected as client {}",
                       parsed->player_name,
                       static_cast<int>(client_id)));

    ByteArray pkg = type_to_byte<Byte>(CONNECTRESPONSE)
        + type_to_byte<std::uint8_t>(client_id)
        + type_to_byte<std::uint32_t>(_server_id);

    send_connectionless(pkg, sender);

  } catch (ClientNotFound&) {
    LOGGER("server", LogLevel::WARNING, "Invalid challenge");
    return;
  }
}

uint32_t NetworkServer::generate_challenge()
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<uint32_t> dis(1, UINT32_MAX);
  return dis(gen);
}

ClientInfo& NetworkServer::find_client_by_endpoint(
    const asio::ip::udp::endpoint& endpoint)
{
  for (auto& client : _clients) {
    if (client.state != ClientState::DISCONNECTED
        && client.endpoint == endpoint)
    {
      return client;
    }
    throw ClientNotFound("");
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new NetworkServer(r, e);
}
}
