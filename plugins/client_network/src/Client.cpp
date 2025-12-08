/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client
*/

#include "NetworkShared.hpp"
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

Client::Client(ClientConnection const& c, SharedQueue<ComponentBuilder> &shared_components, SharedQueue<EventBuilder> &shared_events,  std::atomic<bool> &running) : _socket(_io_c), _components_to_create(std::ref(shared_components)), _events_to_transmit(std::ref(shared_events)),  _running(running)
{
    _socket.open(asio::ip::udp::v4());
    _server_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(c.host), c.port);

    _running.get() = true;
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

  while (_running.get()) {
    try {
      std::error_code ec;
      std::size_t len = recv_buf.read_socket(_socket, sender_endpoint, ec);

      if (len > 0) {
        NETWORK_LOGGER("client",
               LogLevel::DEBUG,
               std::format("received buffer, size : {}", len));
      }

      if (ec) {
        if (_running.get()) {
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
      if (_running.get()) {
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

void Client::transmit_event(EventBuilder &&to_transmit) {
    this->_events_to_transmit.get().lock.lock();
    this->_events_to_transmit.get().queue.push(std::move(to_transmit));
    this->_events_to_transmit.get().lock.unlock();
}
