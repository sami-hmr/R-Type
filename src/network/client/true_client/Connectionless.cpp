#include <system_error>

#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "ServerCommands.hpp"
#include "network/PacketCompresser.hpp"
#include "network/client/Client.hpp"
#include "plugin/Byte.hpp"
#include "plugin/events/ShutdownEvent.hpp"

const std::unordered_map<std::uint8_t, void (Client::*)(ByteArray const&)>
    Client::connectionless_table = {
        {CHALLENGERESPONSE, &Client::handle_challenge_response},
        {CONNECTRESPONSE, &Client::handle_connect_response},
        {DISCONNECT, &Client::handle_disconnect_response},
};

void Client::send(ByteArray const& command, bool hearthbeat)
{
  ByteArray pkg = MAGIC_SEQUENCE + type_to_byte(hearthbeat) + command;

  PacketCompresser::encrypt(pkg);
  try {
    _socket.send_to(asio::buffer(pkg + PROTOCOL_EOF), _server_endpoint);
  } catch (std::system_error const& e) {
    LOGGER_EVTLESS(LogLevel::ERROR,
                   "client",
                   std::format("Failed to send packet: {}", e.what()));
  }
}

void Client::handle_connectionless_response(
    ConnectionlessCommand const& response)
{
  try {
    (this->*(connectionless_table.at(response.command_code)))(response.command);
  } catch (std::out_of_range const&) {
    LOGGER_EVTLESS(LogLevel::DEBUG,
                   "client",
                   std::format("Unhandled connectionless response: {}",
                               response.command_code));
  }
}

void Client::send_getchallenge(int id)
{
  send(type_to_byte<Byte>(GETCHALLENGE) + type_to_byte(id));
}

void Client::send_connect(std::uint32_t challenge)
{
  ByteArray msg = type_to_byte<Byte>(CONNECT) + type_to_byte(challenge)
      + string_to_byte(_player_name);

  send(msg);
}

void Client::handle_challenge_response(ByteArray const& package)
{
  auto const& parsed = parse_challenge_response(package);
  if (!parsed) {
    return;
  }

  LOGGER_EVTLESS(LogLevel::INFO,
                 "client",
                 std::format("Received challenge: {}", parsed->challenge));

  _state = ConnectionState::CONNECTING;
  send_connect(parsed->challenge);
}

void Client::handle_connect_response(ByteArray const& package)
{
  auto const& parsed = parse_connect_response(package);

  if (!parsed) {
    return;
  }

  _state = ConnectionState::CONNECTED;
  LOGGER_EVTLESS(LogLevel::INFO,
                 "client",
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

  LOGGER_EVTLESS(LogLevel::WARNING,
                 "client",
                 std::format("Server disconnected: {}", reason));

  _running.get() = false;

  ShutdownEvent e(std::format("Server disconnected: {}", reason), 0);
  this->transmit_event(EventBuilder("shutdown", e.to_bytes()));
}
