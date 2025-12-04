#include <cstring>
#include <format>
#include <thread>

#include "Network.hpp"

#include "NetworkCommun.hpp"
#include "ServerCommands.hpp"
#include "ecs/Registery.hpp"
#include "plugin/Byte.hpp"
#include "plugin/CircularBuffer.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Events.hpp"

const std::unordered_map<std::uint8_t,
                         void (NetworkClient::*)(ByteArray const&)>
    NetworkClient::_command_table = {
        {CHALLENGERESPONSE, &NetworkClient::handle_challenge_response},
        {CONNECTRESPONSE, &NetworkClient::handle_connect_response},
        {DISCONNECT, &NetworkClient::handle_disconnect_response},
};

NetworkClient::NetworkClient(Registery& r, EntityLoader& l)
    : APlugin(r, l, {}, {})
{
  this->_registery.get().on<ClientConnection>(
      [this](ClientConnection const& c)
      {
        this->_threads.emplace_back([this, c]()
                                    { this->connection_thread(c); });
      });

  this->_registery.get().on<ShutdownEvent>(
      [this](ShutdownEvent const& event)
      {
        _running = false;
        if (_socket) {
          LOGGER("client",
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
          LOGGER("client", LogLevel::DEBUG, "Cleanup requested");
          _socket->close();
        }
      });
}

NetworkClient::~NetworkClient()
{
  _running = false;
  if (_socket) {
    _socket->close();
  }
  for (auto& t : this->_threads) {
    t.join();
  }
}

void NetworkClient::connection_thread(ClientConnection const& c)
{
  try {
    _socket = std::make_unique<asio::ip::udp::socket>(_io_c);
    _socket->open(asio::ip::udp::v4());

    _server_endpoint =
        asio::ip::udp::endpoint(asio::ip::address::from_string(c.host), c.port);

    _player_name = "Player";
    _running = true;
    _state = ConnectionState::DISCONNECTED;

    LOGGER("client",
           LogLevel::INFO,
           std::format("Connecting to {}:{}", c.host, c.port));

    send_getchallenge();
    _state = ConnectionState::CHALLENGING;

    receive_loop();

  } catch (std::exception& e) {
    LOGGER("client",
           LogLevel::ERROR,
           std::format("Connection failed: {}", e.what()));
  }
}

void NetworkClient::receive_loop()
{
  CircularBuffer<BUFFER_SIZE> recv_buf;
  asio::ip::udp::endpoint sender_endpoint;

  while (_running) {
    try {
      std::error_code ec;
      std::size_t len = recv_buf.read_socket(*_socket, sender_endpoint, ec);

      if (len > 0) {
        LOGGER("client",
               LogLevel::DEBUG,
               std::format("received buffer, size : {}", len));
      }

      if (ec == asio::error::would_block) {
        std::this_thread::yield();
        continue;
      }

      if (ec) {
        if (_running) {
          LOGGER("client",
                 LogLevel::ERROR,
                 std::format("Receive error: {}", ec.message()));
        }
        break;
      }

      while (std::optional<ByteArray> p = recv_buf.extract(PROTOCOL_EOF)) {
        LOGGER("client", LogLevel::DEBUG, "package extracted");
        this->handle_package(*p);
      }

    } catch (std::exception& e) {
      if (_running) {
        LOGGER("client",
               LogLevel::ERROR,
               std::format("Error in receive loop: {}", e.what()));
      }
      break;
    }
  }

  LOGGER("client", LogLevel::INFO, "Client receive loop ended");
}

void NetworkClient::handle_package(ByteArray const& package)
{
  std::optional<Package> pkg = this->parse_package(package);

  if (!pkg) {
    return;
  }
  if (pkg->magic != MAGIC_SEQUENCE) {
    LOGGER("client", LogLevel::DEBUG, "Invalid magic sequence, ignoring.");
    return;
  }
  handle_connectionless_response(pkg->real_package);
}

std::optional<Package> NetworkClient::parse_package(ByteArray const& package)
{
  if (package.size() < MAGIC_LENGTH) {
    LOGGER("client", LogLevel::DEBUG, "Package too small");
    return std::nullopt;
  }

  Package pkg;
  std::memcpy(&pkg.magic, package.data(), sizeof(uint32_t));
  pkg.real_package = ByteArray(package.begin() + MAGIC_LENGTH, package.end());

  return pkg;
}

void NetworkClient::send_connectionless(ByteArray const& command)
{
  ByteArray pkg = type_to_byte(MAGIC_SEQUENCE) + command
      + type_to_byte(PROTOCOL_EOF_NUMBER);

  _socket->send_to(asio::buffer(pkg), _server_endpoint);

  LOGGER("client",
         LogLevel::DEBUG,
         std::format("Sent: '{}'", static_cast<int>(command[CMD_INDEX])));
}

void NetworkClient::handle_connectionless_response(ByteArray const& response)
{
  if (response.empty()) {
    LOGGER("client", LogLevel::DEBUG, "Empty response");
    return;
  }

  LOGGER("client",
         LogLevel::DEBUG,
         std::format("Received: '{}'", static_cast<int>(response[CMD_INDEX])));

  std::uint8_t cmd = response[CMD_INDEX];

  auto it = _command_table.find(cmd);
  if (it != _command_table.end()) {
    ByteArray cmd_data(response.begin() + 1, response.end());
    (this->*(it->second))(cmd_data);
  } else {
    LOGGER(
        "client", LogLevel::DEBUG, std::format("Unhandled response: {}", cmd));
  }
}

void NetworkClient::send_getchallenge()
{
  send_connectionless(type_to_byte<Byte>(GETCHALLENGE));
}

void NetworkClient::send_connect(std::uint32_t challenge,
                                 ByteArray const& player_name)
{
  ByteArray msg = type_to_byte<Byte>(CONNECT)
      + type_to_byte<Byte>(CURRENT_PROTOCOL_VERSION) + type_to_byte(challenge)
      + player_name;

  send_connectionless(msg);
}

void NetworkClient::handle_challenge_response(ByteArray const& commandline)
{
  if (commandline.size() < CHALLENGE_SIZE) {
    LOGGER("client",
           LogLevel::WARNING,
           "Invalid challengeResponse: size too small");
    return;
  }

  std::uint32_t challenge = 0;
  std::memcpy(&challenge, commandline.data(), sizeof(uint32_t));

  LOGGER("client",
         LogLevel::INFO,
         std::format("Received challenge: {}", challenge));

  _state = ConnectionState::CONNECTING;
  ByteArray pn(PLAYERNAME_MAX_SIZE, 0);
  std::copy_n(
      _player_name.begin(),
      std::min(_player_name.size(), static_cast<size_t>(PLAYERNAME_MAX_SIZE)),
      pn.begin());
  send_connect(challenge, pn);
}

void NetworkClient::handle_connect_response(ByteArray const& commandline)
{
  if (commandline.size() < (CLIENT_ID_SIZE + SERVER_ID_SIZE)) {
    LOGGER(
        "client", LogLevel::WARNING, "Invalid connectResponse: size too small");
    return;
  }

  std::memcpy(&_client_id, commandline.data(), sizeof(uint8_t));
  std::memcpy(
      &_server_id, commandline.data() + CLIENT_ID_SIZE, sizeof(uint32_t));

  _state = ConnectionState::CONNECTED;

  LOGGER("client",
         LogLevel::INFO,
         std::format("Connected! Client ID: {}, Server ID: {}",
                     static_cast<int>(_client_id),
                     _server_id));
}

void NetworkClient::handle_disconnect_response(ByteArray const& commandline)
{
  std::string reason = "Unknown reason";

  if (!commandline.empty()) {
    // Find the first null terminator or use whole string
    auto null_pos = std::find(commandline.begin(), commandline.end(), 0);
    reason = std::string(commandline.begin(), null_pos);
  }

  LOGGER("client",
         LogLevel::WARNING,
         std::format("Server disconnected: {}", reason));
  _running = false;

  this->_registery.get().emit<ShutdownEvent>(
      std::format("Server disconnected: {}", reason), 0);
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new NetworkClient(r, e);
}
}
