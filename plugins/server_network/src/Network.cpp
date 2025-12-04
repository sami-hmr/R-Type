#include <cstddef>
#include <cstring>
#include <format>
#include <sstream>
#include <functional>

#include "Network.hpp"

#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Events.hpp"

const std::unordered_map<std::uint8_t,
                         void (NetworkServer::*)(
                             const std::string &,
                             const asio::ip::udp::endpoint&)>
    NetworkServer::_command_table = {
        {GETINFO, &NetworkServer::handle_getinfo},
        {GETSTATUS, &NetworkServer::handle_getstatus},
        {GETCHALLENGE, &NetworkServer::handle_getchallenge},
        {CONNECT, &NetworkServer::handle_connect},
};

NetworkServer::NetworkServer(Registery& r, EntityLoader& l) :
 APlugin(r, l, {}, {}), _hostname("R-Type Server"), _mapname("level1")
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

    _socket->non_blocking(true);
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

asio::socket_base::message_flags NetworkServer::handle_receive(const boost::system::error_code& UNUSED error, std::size_t UNUSED bytes_transferred)
{
  asio::socket_base::message_flags flag = MSG_OOB;
  std::cout << "handled the reception" << std::endl;
  return flag;
}

void NetworkServer::receive_loop()
{
  std::array<char, BUFFER_SIZE> recv_buf {};
  asio::ip::udp::endpoint sender_endpoint;

  // std::cout << "hell yeah" << std::endl;
  //         _socket->async_receive_from(
  //               asio::buffer(recv_buf), sender_endpoint,
  //               NetworkServer::handle_receive);
  // std::cout << "hell nah" << std::endl;

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
          LOGGER("server",
                 LogLevel::ERROR,
                 std::format("Receive error: {}", ec.message()));
        }
        break;
      }

      if (len < MAGIC_LENGTH) {
        continue;
      }

      uint32_t magic = 0;
      std::memcpy(&magic, recv_buf.data(), sizeof(uint32_t));

      if (magic == MAGIC_SEQUENCE) {
        std::string command(recv_buf.data() + MAGIC_LENGTH, len - MAGIC_LENGTH);
        if (!command.empty() && command.back() == END_OF_CMD) {
          command.pop_back();
        }
        handle_connectionless_packet(command, sender_endpoint);
      } else {
        LOGGER("server", LogLevel::DEBUG, "Received connected packet");
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

void NetworkServer::handle_connectionless_packet(
    const std::string& command, const asio::ip::udp::endpoint& sender)
{
  LOGGER(
      "server", LogLevel::DEBUG, std::format("Received connectionless packet: '{}'", int(command[CMD_INDEX])));

  std::uint8_t cmd = command[CMD_INDEX];

  auto it = _command_table.find(cmd);
  if (it != _command_table.end()) {
    (this->*(it->second))(command, sender);
  } else {
    LOGGER(
        "server", LogLevel::WARNING, std::format("Unknown command: {}", cmd));
  }
}

void NetworkServer::send_connectionless(const std::string& response,
                                        const asio::ip::udp::endpoint& endpoint)
{
  std::size_t length = response.size();
  std::vector<char> packet;
  packet.resize(MAGIC_LENGTH + length + 1);

  uint32_t magic = MAGIC_SEQUENCE;
  std::memcpy(packet.data(), &magic, sizeof(uint32_t));

  std::memcpy(packet.data() + MAGIC_LENGTH, response.c_str(), length);
  packet[MAGIC_LENGTH + length] = END_OF_CMD;

  _socket->send_to(asio::buffer(packet), endpoint);

  LOGGER("server", LogLevel::DEBUG, std::format("Sent: '{}'", static_cast<int>(response[CMD_INDEX])));
}

void NetworkServer::handle_getinfo(const std::string& UNUSED commandline,
                                   const asio::ip::udp::endpoint& sender)
{
  if (commandline.size() != COMMAND_SIZE) {
    LOGGER("server", LogLevel::WARNING, "Invalid getinfo command");
    return;
  }
  std::ostringstream oss;
  oss << INFORESPONSE << std::string(_hostname.begin(), _hostname.end()) << std::string(_mapname.begin(), _mapname.end())
      << COOP << _max_players << CURRENT_PROTOCOL_VERSION;

  send_connectionless(oss.str(), sender);
}

void NetworkServer::handle_getstatus(const std::string& UNUSED commandline,
                                     const asio::ip::udp::endpoint& sender)
{
  if (commandline.size() != COMMAND_SIZE) {
    LOGGER("server", LogLevel::WARNING, "Invalid getstatus command");
    return;
  }
  std::ostringstream oss;
  oss << STATUSRESPONSE << std::string(_hostname.begin(), _hostname.end()) << std::string(_mapname.begin(), _mapname.end())
      << COOP << _max_players << CURRENT_PROTOCOL_VERSION;

  for (const auto& client : _clients) {
    if (client.state == ClientState::CONNECTED) {
      oss << client.score << client.ping <<
        std::string(client.player_name.begin(), client.player_name.end());
    }
  }

  send_connectionless(oss.str(), sender);
}

void NetworkServer::handle_getchallenge(const std::string& UNUSED commandline,
                                        const asio::ip::udp::endpoint& sender)
{
  if (commandline.size() != COMMAND_SIZE) {
    LOGGER("server", LogLevel::WARNING, "Invalid getchallenge command");
    return;
  }
  uint32_t challenge = generate_challenge();

  ClientInfo* client = find_client_by_endpoint(sender);
  if (client == nullptr) {
    for (auto& c : _clients) {
      if (c.state == ClientState::DISCONNECTED) {
        client = &c;
        break;
      }
    }
  }

  if (client != nullptr) {
    client->endpoint = sender;
    client->challenge = challenge;
    client->state = ClientState::CHALLENGING;
  }

  std::ostringstream oss;
  oss << CHALLENGERESPONSE << challenge;
  send_connectionless(oss.str(), sender);
}

std::vector<std::string> NetworkServer::parse_connect_args(const std::string& commandline)
{
  std::vector<std::string> args;

  args.push_back(commandline.substr(0, PROTOCOL_SIZE));
  args.push_back(commandline.substr(PROTOCOL_SIZE, CHALLENGE_SIZE));
  args.push_back(commandline.substr(PROTOCOL_SIZE + CHALLENGE_SIZE, PLAYERNAME_MAX_SIZE));
  return args;
}

void NetworkServer::handle_connect(const std::string& commandline,
                                   const asio::ip::udp::endpoint& sender)
{
  std::vector<std::string> args = parse_connect_args(commandline);
  std::size_t len = args.size();

  if (len != CONNECT_CMD_SIZE) {
    LOGGER("server", LogLevel::WARNING, std::format("Invalid connect command: command size is {} and content is {}", len, &commandline[0] + 1));
    return;
  }
    // LOGGER("server", LogLevel::WARNING, std::format("Look at that '{}' '{}' '{}' '{}' '{}' '{}'", (args[PROTOCOL_SIZE]), args[PROTOCOL_SIZE].size(), (args[CHALLENGE_CNT_INDEX]), args[CHALLENGE_CNT_INDEX].size(), args[PLAYERNAME_CNT_INDEX], args[PLAYERNAME_CNT_INDEX].size()));
  int protocol_version = std::stoi(args[PROTOCOL_INDEX]);
  uint32_t challenge = std::stoul(args[CHALLENGE_CNT_INDEX]);
  std::string player_name = args[PLAYERNAME_CNT_INDEX];
  std::ostringstream oss;

  if (protocol_version != CURRENT_PROTOCOL_VERSION) {
    std::array<char, ERROR_MSG_SIZE> msg {};
    std::string error_message = "Protocol version mismatch";
    std::copy(error_message.data(), error_message.data() + error_message.length(), msg.begin());
    oss << DISCONNECT << std::string(msg.begin(), msg.end());
    send_connectionless(oss.str(), sender);
    return;
  }

  ClientInfo* client = find_client_by_endpoint(sender);
  if (client == nullptr || client->state != ClientState::CHALLENGING
      || client->challenge != challenge)
  {
    LOGGER("server", LogLevel::WARNING, "Invalid challenge");
    return;
  }

  uint8_t client_id = 0;
  for (size_t i = 0; i < _clients.size(); ++i) {
    if (&_clients.at(i) == client) {
      client_id = static_cast<uint8_t>(i);
      break;
    }
  }

  client->client_id = client_id;
  std::copy(player_name.data(), player_name.data() + player_name.length(), client->player_name.begin());
  client->state = ClientState::CONNECTED;

  LOGGER("server", LogLevel::INFO,
    std::format("Player '{}' connected as client {}",
      player_name, static_cast<int>(client_id)));

  oss << CONNECTRESPONSE << static_cast<char>(client_id) << _server_id;
  send_connectionless(oss.str(), sender);
}

uint32_t NetworkServer::generate_challenge()
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<uint32_t> dis(1, UINT32_MAX);
  return dis(gen);
}

ClientInfo* NetworkServer::find_client_by_endpoint(
    const asio::ip::udp::endpoint& endpoint)
{
  for (auto& client : _clients) {
    if (client.state != ClientState::DISCONNECTED
        && client.endpoint == endpoint)
    {
      return &client;
    }
  }
  return nullptr;
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new NetworkServer(r, e);
}
}
