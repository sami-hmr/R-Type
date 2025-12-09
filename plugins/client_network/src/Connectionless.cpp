#include "Client.hpp"
#include "NetworkShared.hpp"
#include "ServerCommands.hpp"
#include "plugin/Byte.hpp"
#include "plugin/events/ShutdownEvent.hpp"

const std::unordered_map<std::uint8_t, void (Client::*)(ByteArray const&)>
    Client::connectionless_table = {
        {CHALLENGERESPONSE, &Client::handle_challenge_response},
        {CONNECTRESPONSE, &Client::handle_connect_response},
        {DISCONNECT, &Client::handle_disconnect_response},
};

void Client::send_connectionless(ByteArray const& command)
{
  ByteArray pkg = MAGIC_SEQUENCE + command + PROTOCOL_EOF;

  _socket.send_to(asio::buffer(pkg), _server_endpoint);

  NETWORK_LOGGER(
      "client",
      LogLevel::DEBUG,
      std::format("Sent connectionless package of size: {}", pkg.size()));
}

void Client::handle_connectionless_response(
    ConnectionlessCommand const& response)
{
  try {
    (this->*(connectionless_table.at(response.command_code)))(response.command);
  } catch (std::out_of_range const&) {
    NETWORK_LOGGER("client",
                   LogLevel::DEBUG,
                   std::format("Unhandled connectionless response: {}",
                               response.command_code));
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
  auto const& parsed = parse_challenge_response(package);
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
  auto const& parsed = parse_connect_response(package);

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

  _running.get() = false;

  ShutdownEvent e(std::format("Server disconnected: {}", reason), 0);
  this->transmit_event(
      EventBuilder {.event_id = "shutdown", .data = e.to_bytes()});
}
