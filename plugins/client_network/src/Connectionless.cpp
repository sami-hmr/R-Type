#include "Client.hpp"
#include "plugin/Byte.hpp"

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

  _running.get() = false;

  // this->_registery.get().emit<ShutdownEvent>(
  //     std::format("Server disconnected: {}", reason), 0);
  // no registery in client. Find anothere way to disconnect
}
