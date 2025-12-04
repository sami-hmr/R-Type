#include <chrono>
#include <cstring>
#include <format>
#include <sstream>
#include <thread>

#include "Network.hpp"

#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Events.hpp"

const std::unordered_map<std::uint8_t,
                         void (NetworkClient::*)(
                             const std::string &)>
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
  std::array<char, BUFFER_SIZE> recv_buf {};
  asio::ip::udp::endpoint sender_endpoint;

  while (_running) {
    try {
      std::error_code ec;
      size_t len =
          _socket->receive_from(asio::buffer(recv_buf), sender_endpoint, 0, ec);

      if (ec == asio::error::would_block) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION));
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

      if (len >= MAGIC_LENGTH) {
        uint32_t magic = 0;
        std::memcpy(&magic, recv_buf.data(), sizeof(uint32_t));

        if (magic == MAGIC_SEQUENCE) {
          std::string response(recv_buf.data() + MAGIC_LENGTH, len - MAGIC_LENGTH);
          if (!response.empty() && response.back() == END_OF_CMD) {
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
  packet.resize(MAGIC_LENGTH + command.size() + 1);

  uint32_t magic = MAGIC_SEQUENCE;
  std::memcpy(packet.data(), &magic, sizeof(uint32_t));

  std::memcpy(packet.data() + MAGIC_LENGTH, command.c_str(), command.size());
  packet[MAGIC_LENGTH + command.size()] = END_OF_CMD;

  _socket->send_to(asio::buffer(packet), _server_endpoint);

  LOGGER("client", LogLevel::DEBUG, std::format("Sent: '{}'", static_cast<int>(command[CMD_INDEX])));
}

void NetworkClient::handle_connectionless_response(const std::string& response)
{
  LOGGER("client", LogLevel::DEBUG, std::format("Received: '{}'", static_cast<int>(response[CMD_INDEX])));

  std::uint8_t cmd = response[CMD_INDEX];

  auto it = _command_table.find(cmd);
  if (it != _command_table.end()) {
    (this->*(it->second))(response);
  } else {
    LOGGER(
        "client", LogLevel::DEBUG, std::format("Unhandled response: {}", cmd));
  }
}

void NetworkClient::send_getchallenge()
{
  std::ostringstream oss;
  oss << GETCHALLENGE;
  send_connectionless(oss.str());
}

void NetworkClient::send_connect(std::uint32_t challenge, const std::string& player_name)
{
  std::ostringstream oss;
  oss << CONNECT << CURRENT_PROTOCOL_VERSION << challenge << player_name;
  send_connectionless(oss.str());
}

bool NetworkClient::is_valid_response_format(const std::vector<std::string> &args, std::uint8_t requested_size, const std::string &err_mess)
{
  std::size_t len = args.size();

  if (len != requested_size) {
    LOGGER("client", LogLevel::WARNING, err_mess.c_str());
    return false;
  }
  return true;
}

std::vector<std::string> NetworkClient::parse_challenge_response(const std::string& resp)
{
  std::vector<std::string> args;

  args.push_back(resp.substr(0, MAGIC_LENGTH));
  args.push_back(resp.substr(MAGIC_LENGTH, PROTOCOL_SIZE));
  args.push_back(resp.substr(MAGIC_LENGTH + PROTOCOL_SIZE, CHALLENGE_SIZE));
  args.push_back(resp.substr(resp.length() - 1, 1));
  return args;
}

void NetworkClient::handle_challenge_response(
    const std::string& commandline)
{
  std::vector<std::string> args = parse_challenge_response(commandline);

  if (!is_valid_response_format(args, CHALLENGE_RESP_CMD_SIZE, "Invalid challengeResponse")) {
    return;
  }

  std::uint32_t challenge = std::stoul(args[CHALLENGE_CLG_INDEX]);
  LOGGER("client", LogLevel::INFO,
    std::format("Received challenge: {}", challenge));

  _state = ConnectionState::CONNECTING;
  send_connect(challenge, _player_name);
}

std::vector<std::string> NetworkClient::parse_connect_response(const std::string& resp)
{
  std::vector<std::string> args;

  args.push_back(resp.substr(0, MAGIC_LENGTH));
  args.push_back(resp.substr(MAGIC_LENGTH, PROTOCOL_SIZE));
  args.push_back(resp.substr(MAGIC_LENGTH + PROTOCOL_SIZE, CLIENT_ID_SIZE));
  args.push_back(resp.substr(MAGIC_LENGTH + PROTOCOL_SIZE + CLIENT_ID_SIZE,
    SERVER_ID_SIZE));
  args.push_back(resp.substr(resp.length() - 1, 1));
  return args;
}

void NetworkClient::handle_connect_response(
    const std::string& commandline)
{
  std::vector<std::string> args = parse_connect_response(commandline);

  if (!is_valid_response_format(args, CONNECT_RESP_CMD_SIZE, "Invalid connectResponse")) {
    return;
  }
  _client_id = static_cast<std::uint8_t>(std::stoi(args[CLIENT_ID_CNT_RESP_INDEX]));
  _server_id = std::stoul(args[SERVER_ID_CNT_RESP_INDEX]);
  _state = ConnectionState::CONNECTED;

  LOGGER("client", LogLevel::INFO,
   std::format("Connected! Client ID: {}, Server ID: {}",
    static_cast<int>(_client_id), _server_id));
}

std::vector<std::string> NetworkClient::parse_disconnect_response(const std::string& resp)
{
  std::vector<std::string> args;

  args.push_back(resp.substr(0, MAGIC_LENGTH));
  args.push_back(resp.substr(MAGIC_LENGTH, PROTOCOL_SIZE));
  args.push_back(resp.substr(MAGIC_LENGTH + PROTOCOL_SIZE, ERROR_MSG_SIZE));
  args.push_back(resp.substr(resp.length() - 1, 1));
  return args;
}

void NetworkClient::handle_disconnect_response(
    const std::string& commandline)
{
  std::vector<std::string> args = parse_disconnect_response(commandline);

  if (!is_valid_response_format(args, DISCONNECT_RESP_CMD_SIZE, "Invalid disconnectResponse")) {
    return;
  }
  std::string reason = args[ERR_MESS_DSCNT_INDEX][0] == END_OF_CMD ?
    args[ERR_MESS_DSCNT_INDEX] : "Unknown reason";
  LOGGER("client", LogLevel::WARNING,
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
