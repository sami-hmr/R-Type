#include <cstring>
#include <format>
#include <sstream>

#include "Network.hpp"

#include "Events.hpp"
#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"

const std::unordered_map<std::string,
                         void (NetworkServer::*)(
                             const std::vector<std::string>&,
                             const asio::ip::udp::endpoint&)>
    NetworkServer::_command_table = {
        {"getinfo", &NetworkServer::handle_getinfo},
        {"getstatus", &NetworkServer::handle_getstatus},
        {"getchallenge", &NetworkServer::handle_getchallenge},
        {"connect", &NetworkServer::handle_connect},
};

NetworkServer::NetworkServer(Registery& r, EntityLoader& l)
    : APlugin(r, l, {}, {})
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

void NetworkServer::receive_loop()
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
          LOGGER("server",
                 LogLevel::ERROR,
                 std::format("Receive error: {}", ec.message()));
        }
        break;
      }

      if (len < 4) {
        continue;
      }

      uint32_t magic = 0;
      std::memcpy(&magic, recv_buf.data(), sizeof(uint32_t));

      if (magic == MAGIC_SEQUENCE) {
        std::string command(recv_buf.data() + 4, len - 4);
        if (!command.empty() && command.back() == '\0') {
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
      "server", LogLevel::DEBUG, std::format("Connectionless: '{}'", command));

  std::vector<std::string> args = parse_args(command);
  if (args.empty()) {
    return;
  }

  const std::string& cmd = args[0];

  auto it = _command_table.find(cmd);
  if (it != _command_table.end()) {
    (this->*(it->second))(args, sender);
  } else {
    LOGGER(
        "server", LogLevel::WARNING, std::format("Unknown command: {}", cmd));
  }
}

void NetworkServer::send_connectionless(const std::string& response,
                                        const asio::ip::udp::endpoint& endpoint)
{
  std::vector<char> packet;
  packet.resize(4 + response.size() + 1);

  uint32_t magic = MAGIC_SEQUENCE;
  std::memcpy(packet.data(), &magic, sizeof(uint32_t));

  std::memcpy(packet.data() + 4, response.c_str(), response.size());
  packet[4 + response.size()] = '\0';

  _socket->send_to(asio::buffer(packet), endpoint);

  LOGGER("server", LogLevel::DEBUG, std::format("Sent: '{}'", response));
}

void NetworkServer::handle_getinfo(const std::vector<std::string>& args,
                                   const asio::ip::udp::endpoint& sender)
{
  std::ostringstream oss;
  oss << "infoResponse;hostname;" << _hostname << ";mapname;" << _mapname
      << ";gametype;coop"
      << ";maxplayers;" << _max_players << ";protocol;1";

  send_connectionless(oss.str(), sender);
}

void NetworkServer::handle_getstatus(const std::vector<std::string>& args,
                                     const asio::ip::udp::endpoint& sender)
{
  std::ostringstream oss;
  oss << "statusResponse;hostname;" << _hostname << ";maxplayers;"
      << _max_players;

  for (const auto& client : _clients) {
    if (client.state == ClientState::CONNECTED) {
      oss << ";0;0;" << client.player_name;
    }
  }

  send_connectionless(oss.str(), sender);
}

void NetworkServer::handle_getchallenge(const std::vector<std::string>& args,
                                        const asio::ip::udp::endpoint& sender)
{
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
  oss << "challengeResponse;" << challenge;
  send_connectionless(oss.str(), sender);
}

void NetworkServer::handle_connect(const std::vector<std::string>& args,
                                   const asio::ip::udp::endpoint& sender)
{
  if (args.size() < 4) {
    LOGGER("server", LogLevel::WARNING, "Invalid connect command");
    return;
  }

  int protocol_version = std::stoi(args[1]);
  uint32_t challenge = std::stoul(args[2]);
  std::string player_name = args[3];

  if (protocol_version != 1) {
    send_connectionless("disconnect;Protocol version mismatch", sender);
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
  client->player_name = player_name;
  client->state = ClientState::CONNECTED;

  LOGGER("server",
         LogLevel::INFO,
         std::format("Player '{}' connected as client {}",
                     player_name,
                     static_cast<int>(client_id)));

  std::ostringstream oss;
  oss << "connectResponse;" << static_cast<int>(client_id) << ";" << _server_id;
  send_connectionless(oss.str(), sender);
}

uint32_t NetworkServer::generate_challenge()
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<uint32_t> dis(1, UINT32_MAX);
  return dis(gen);
}

std::vector<std::string> NetworkServer::parse_args(const std::string& command)
{
  std::vector<std::string> args;
  std::stringstream ss(command);
  std::string arg;

  while (std::getline(ss, arg, ';')) {
    args.push_back(arg);
  }

  return args;
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
