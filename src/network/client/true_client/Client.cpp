/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client
*/

#include "network/client/Client.hpp"

#include <asio/ip/address.hpp>

#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "plugin/CircularBuffer.hpp"

Client::Client(ClientConnection const& c,
               SharedQueue<ComponentBuilder>& shared_components,
               SharedQueue<EventBuilder>& shared_events,
               SharedQueue<EventBuilder>& shared_exec_events,
               std::atomic<bool>& running)
    : _socket(_io_c)
    , _components_to_create(std::ref(shared_components))
    , _events_to_transmit(std::ref(shared_events))
    , _event_to_exec(std::ref(shared_exec_events))
    , _running(running)
{
  _socket.open(asio::ip::udp::v4());
  _server_endpoint =
      asio::ip::udp::endpoint(asio::ip::address::from_string(c.host), c.port);

  NETWORK_LOGGER("client",
                 LogLevel::INFO,
                 std::format("Connecting to {}:{}", c.host, c.port));
  this->_queue_reader = std::thread([this]() { this->send_evt(); });
}

void Client::close()
{
  this->_running.get() = false;
  this->_events_to_transmit.get().release();
  if (this->_queue_reader.joinable()) {
    this->_queue_reader.join();
  }
  if (_socket.is_open()) {
    _socket.close();
  }
}

Client::~Client()
{
  this->_running.get() = false;
  this->_events_to_transmit.get().release();
  if (this->_queue_reader.joinable()) {
    this->_queue_reader.join();
  }
  if (_socket.is_open()) {
    _socket.close();
  }
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
        // NETWORK_LOGGER("client",
        //                LogLevel::DEBUG,
        //                std::format("received buffer, size : {}", len));
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
        // NETWORK_LOGGER("client", LogLevel::DEBUG, "package extracted");
        // std::cout << "[";
        // for (auto i : *p) {
        //     std::cout << " " << (unsigned int)i << ",";
        // }
        // std::cout << "]\n";

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
  std::optional<Package> pkg = parse_package(package);

  if (!pkg) {
    return;
  }
  if (pkg->magic != MAGIC_SEQUENCE) {
    NETWORK_LOGGER(
        "client", LogLevel::DEBUG, "Invalid magic sequence, ignoring.");
    return;
  }
  if (this->_state == ConnectionState::CONNECTED) {
    auto const& parsed = parse_connected_package(pkg->real_package);
    if (!parsed) {
      return;
    }
    this->handle_connected_package(parsed.value());
  } else {
    auto const& parsed = parse_connectionless_package(pkg->real_package);
    if (!parsed) {
      return;
    }
    this->handle_connectionless_response(parsed.value());
  }
}
