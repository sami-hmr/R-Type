/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client
*/

#include "network/client/Client.hpp"

#include <asio/ip/address.hpp>
#include <asio/ip/udp.hpp>
#include <asio/registered_buffer.hpp>

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
    , _last_ping(std::chrono::steady_clock::now().time_since_epoch().count())
{
  _socket.open(asio::ip::udp::v4());
  _socket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), 0));
  _client_endpoint = asio::ip::udp::endpoint(_socket.local_endpoint());
  _server_endpoint =
      asio::ip::udp::endpoint(asio::ip::address::from_string(c.host), c.port);

  LOGGER_EVTLESS(LogLevel::INFO,
                 "client",
                 std::format("Connecting to {}:{}", c.host, c.port));
  this->_queue_reader = std::thread(&Client::send_evt, this);
  this->_hearthbeat = std::thread(&Client::send_hearthbeat, this);
}

void Client::close()
{
  this->_running.get() = false;
  this->_events_to_transmit.get().release();
  if (this->_queue_reader.joinable()) {
    this->_queue_reader.join();
  }
}

Client::~Client()
{
  this->_running.get() = false;
  this->_events_to_transmit.get().release();
  if (this->_queue_reader.joinable()) {
    this->_queue_reader.join();
  }
  if (this->_hearthbeat.joinable()) {
    this->_hearthbeat.join();
  }
  // this->_socket.send_to(asio::buffer(""), this->_client_endpoint);
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
        // LOGGER_EVTLESS("client",
        //                LogLevel::DEBUG,
        //                std::format("received buffer, size : {}", len));
      }

      if (ec) {
        if (_running.get()) {
          LOGGER_EVTLESS(LogLevel::ERROR,
                         "client",
                         std::format("Receive error: {}", ec.message()));
        }
        continue;
      }

      while (std::optional<ByteArray> p = recv_buf.extract(PROTOCOL_EOF)) {
        // LOGGER_EVTLESSGER("client", LogLevel::DEBUG, "package extracted");
        // std::cout << "[";
        // for (auto i : *p) {
        //     std::cout << " " << (unsigned int)i << ",";
        // }
        // std::cout << "]\n";

        this->handle_package(*p);
      }
    } catch (CustomException& e) {
      LOGGER_EVTLESS(
          LogLevel::ERROR,
          "server",
          std::format(
              "Error in receive loop: {}: ", e.what(), e.format_context()));
      break;
    }

    catch (std::exception& e)
    {
      if (_running.get()) {
        LOGGER_EVTLESS(LogLevel::ERROR,
                       "client",
                       std::format("Error in receive loop: {}", e.what()));
      }
      break;
    }
  }

  LOGGER_EVTLESS(LogLevel::INFO, "client", "Client receive loop ended");
}

void Client::handle_package(ByteArray const& package)
{
  std::optional<Package> pkg = parse_package(package);

  if (!pkg) {
    return;
  }
  if (pkg->magic != MAGIC_SEQUENCE) {
    LOGGER_EVTLESS(
        LogLevel::DEBUG, "client", "Invalid magic sequence, ignoring.");
    return;
  }
  this->_last_ping =
      std::chrono::steady_clock::now().time_since_epoch().count();
  if (pkg->hearthbeat) {
    this->handle_hearthbeat(pkg->real_package);
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

bool Client::should_disconnect() const
{
  std::size_t now = std::chrono::steady_clock::now().time_since_epoch().count();
  return (this->_last_ping + disconnection_timeout) < now;
}
