/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client
*/

#include "plugin/CircularBuffer.hpp"
#include "plugin/events/Events.hpp"
#include "Client.hpp"

const std::unordered_map<std::uint8_t,
                         void (Client::*)(ByteArray const&)>
    Client::_command_table = {
        {CHALLENGERESPONSE, &Client::handle_challenge_response},
        {CONNECTRESPONSE, &Client::handle_connect_response},
        {DISCONNECT, &Client::handle_disconnect_response},
};

Client::Client(ClientConnection const& c, std::queue<std::shared_ptr<ByteArray>> &cmpnts, std::atomic<bool> &running, std::mutex &lock) : _socket(_io_c), _cmpts_lock(lock), _running(running), _components_to_create(std::reference_wrapper(cmpnts))
{
    _socket.open(asio::ip::udp::v4());
    _server_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(c.host), c.port);

    _running = true;
    NETWORK_LOGGER("client",
           LogLevel::INFO,
           std::format("Connecting to {}:{}", c.host, c.port));
}

void Client::close()
{
  if (_socket.is_open()) {
    _socket.close();
  }
}

Client::~Client()
{
  _socket.close();
}

void Client::connect()
{
    send_getchallenge();
    _state = ConnectionState::CHALLENGING;
    receive_loop();
}

void Client::receive_loop()
{
  CircularBuffer<BUFFER_SIZE> recv_buf;
  asio::ip::udp::endpoint sender_endpoint;

  while (_running) {
    try {
      std::error_code ec;
      std::size_t len = recv_buf.read_socket(_socket, sender_endpoint, ec);

      if (len > 0) {
        NETWORK_LOGGER("client",
               LogLevel::DEBUG,
               std::format("received buffer, size : {}", len));
      }

      if (ec) {
        if (_running) {
          NETWORK_LOGGER("client",
                 LogLevel::ERROR,
                 std::format("Receive error: {}", ec.message()));
        }
        break;
      }

      while (std::optional<ByteArray> p = recv_buf.extract(PROTOCOL_EOF)) {
        NETWORK_LOGGER("client", LogLevel::DEBUG, "package extracted");
        this->handle_package(*p);
      }

    } catch (std::exception& e) {
      if (_running) {
        NETWORK_LOGGER("client",
               LogLevel::ERROR,
               std::format("Error in receive loop: {}", e.what()));
      }
      break;
    }
  }

  NETWORK_LOGGER("client", LogLevel::INFO, "Client receive loop ended");
}

void Client::handle_package(ByteArray const& package)
{
  std::optional<Package> pkg = this->parse_package(package);

  if (!pkg) {
    return;
  }
  if (pkg->magic != MAGIC_SEQUENCE) {
    NETWORK_LOGGER("client", LogLevel::DEBUG, "Invalid magic sequence, ignoring.");
    return;
  }
  handle_connectionless_response(pkg->real_package);
}

void Client::send_connectionless(ByteArray const& command)
{
  ByteArray pkg = MAGIC_SEQUENCE + command + PROTOCOL_EOF;

  _socket.send_to(asio::buffer(pkg), _server_endpoint);

  NETWORK_LOGGER("client",
         LogLevel::DEBUG,
         std::format("Sent connectionless package of size: {}", pkg.size()));
}

void Client::handle_connectionless_response(ByteArray const& response)
{
  if (response.empty()) {
    NETWORK_LOGGER("client", LogLevel::DEBUG, "Empty response");
    return;
  }

  NETWORK_LOGGER("client",
         LogLevel::DEBUG,
         std::format("Received connectionless response of size: {}",
                     response.size()));
  auto const& parsed = this->parse_connectionless_package(response);
  if (!parsed) {
    return;
  }
  try {
    (this->*(_command_table.at(parsed->command_code)))(parsed->command);
  } catch (std::out_of_range const&) {
    NETWORK_LOGGER("client", LogLevel::DEBUG,
        std::format("Unhandled connectionless response: {}", parsed->command_code));
  }
}

void Client::send_getchallenge()
{
  send_connectionless(type_to_byte<Byte>(GETCHALLENGE));
}

void Client::send_connect(std::uint32_t challenge)
{
  ByteArray msg = type_to_byte<Byte>(CONNECT) + type_to_byte(challenge)
      + string_to_byte(_player_name);

  send_connectionless(msg);
}

void Client::handle_challenge_response(ByteArray const& package)
{
  auto const& parsed = this->parse_challenge_response(package);
  if (!parsed) {
    return;
  }

  NETWORK_LOGGER("client",
         LogLevel::INFO,
         std::format("Received challenge: {}", parsed->challenge));

  _state = ConnectionState::CONNECTING;
  send_connect(parsed->challenge);
}

void Client::handle_connect_response(ByteArray const& package)
{
  auto const& parsed = this->parse_connect_response(package);

  if (!parsed) {
    return;
  }

  _state = ConnectionState::CONNECTED;
  NETWORK_LOGGER("client",
         LogLevel::INFO,
         std::format("Connected! Client ID: {}, Server ID: {}",
                     parsed->client_id,
                     parsed->server_id));
}

void Client::handle_disconnect_response(ByteArray const& package)
{
  std::string reason = "Unknown reason";

  if (!package.empty()) {
    // Find the first null terminator or use whole string
    auto null_pos = std::find(package.begin(), package.end(), 0);
    reason = std::string(package.begin(), null_pos);
  }

  NETWORK_LOGGER("client",
         LogLevel::WARNING,
         std::format("Server disconnected: {}", reason));
  
  _running = false;

  // this->_registery.get().emit<ShutdownEvent>(
  //     std::format("Server disconnected: {}", reason), 0);
  // no registery in client. Find anothere way to disconnect
}
