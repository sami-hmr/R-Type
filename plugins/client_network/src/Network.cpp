#include <chrono>
#include <cstring>
#include <format>
#include <sstream>
#include <thread>

#include "Network.hpp"

#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Events.hpp"

const std::unordered_map<std::string,
                         void (NetworkClient::*)(
                             const std::vector<std::string>&)>
    NetworkClient::_command_table = {
        {"challengeResponse", &NetworkClient::handle_challenge_response},
        {"connectResponse", &NetworkClient::handle_connect_response},
        {"disconnect", &NetworkClient::handle_disconnect_response},
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

    _socket->non_blocking(true);

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
  std::array<char, 2048> recv_buf {};
  asio::ip::udp::endpoint sender_endpoint;

  while (_running) {
    try {
      std::error_code ec;
      size_t len =
          _socket->receive_from(asio::buffer(recv_buf), sender_endpoint, 0, ec);

      if (ec == asio::error::would_block) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

      if (len >= 4) {
        uint32_t magic = 0;
        std::memcpy(&magic, recv_buf.data(), sizeof(uint32_t));

        if (magic == MAGIC_SEQUENCE) {
          std::string response(recv_buf.data() + 4, len - 4);
          if (!response.empty() && response.back() == '\0') {
            response.pop_back();
          }
          handle_connectionless_response(response);
        } else {
          LOGGER("client", LogLevel::DEBUG, "Received connected packet");
        }
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

void NetworkClient::send_connectionless(const std::string& command)
{
  std::vector<char> packet;
  packet.resize(4 + command.size() + 1);

  uint32_t magic = MAGIC_SEQUENCE;
  std::memcpy(packet.data(), &magic, sizeof(uint32_t));

  std::memcpy(packet.data() + 4, command.c_str(), command.size());
  packet[4 + command.size()] = '\0';

  _socket->send_to(asio::buffer(packet), _server_endpoint);

  LOGGER("client", LogLevel::DEBUG, std::format("Sent: '{}'", command));
}

void NetworkClient::handle_connectionless_response(const std::string& response)
{
  LOGGER("client", LogLevel::DEBUG, std::format("Received: '{}'", response));

  std::vector<std::string> args = parse_args(response);
  if (args.empty()) {
    return;
  }

  const std::string& cmd = args[0];

  auto it = _command_table.find(cmd);
  if (it != _command_table.end()) {
    (this->*(it->second))(args);
  } else {
    LOGGER(
        "client", LogLevel::DEBUG, std::format("Unhandled response: {}", cmd));
  }
}

void NetworkClient::send_getchallenge()
{
  send_connectionless("getchallenge");
}

void NetworkClient::send_connect(uint32_t challenge,
                                 const std::string& player_name)
{
  std::ostringstream oss;
  oss << "connect;1;" << challenge << ";" << player_name;
  send_connectionless(oss.str());
}

void NetworkClient::handle_challenge_response(
    const std::vector<std::string>& args)
{
  if (args.size() < 2) {
    LOGGER("client", LogLevel::WARNING, "Invalid challengeResponse");
    return;
  }

  uint32_t challenge = std::stoul(args[1]);
  LOGGER("client",
         LogLevel::INFO,
         std::format("Received challenge: {}", challenge));

  _state = ConnectionState::CONNECTING;
  send_connect(challenge, _player_name);
}

void NetworkClient::handle_connect_response(
    const std::vector<std::string>& args)
{
  if (args.size() < 3) {
    LOGGER("client", LogLevel::WARNING, "Invalid connectResponse");
    return;
  }

  _client_id = static_cast<uint8_t>(std::stoi(args[1]));
  _server_id = std::stoul(args[2]);
  _state = ConnectionState::CONNECTED;

  LOGGER("client",
         LogLevel::INFO,
         std::format("Connected! Client ID: {}, Server ID: {}",
                     static_cast<int>(_client_id),
                     _server_id));
}

void NetworkClient::handle_disconnect_response(
    const std::vector<std::string>& args)
{
  std::string reason = args.size() > 1 ? args[1] : "Unknown reason";
  LOGGER("client",
         LogLevel::WARNING,
         std::format("Server disconnected: {}", reason));
  _running = false;

  this->_registery.get().emit<ShutdownEvent>(
      std::format("Server disconnected: {}", reason), 0);
}

std::vector<std::string> NetworkClient::parse_args(const std::string& response)
{
  std::vector<std::string> args;
  std::stringstream ss(response);
  std::string arg;

  while (std::getline(ss, arg, ';')) {
    args.push_back(arg);
  }

  return args;
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new NetworkClient(r, e);
}
}
