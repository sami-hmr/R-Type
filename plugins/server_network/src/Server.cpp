/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Server
*/
#include <atomic>
#include <queue>
#include <semaphore>

#include "Server.hpp"
#include <asio/system_error.hpp>

#include "Network.hpp"
#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "plugin/Byte.hpp"

// #include "plugin/events/Events.hpp"

Server::Server(ServerLaunching const& s,
               SharedQueue<ComponentBuilder>& comp_queue,
               SharedQueue<EventBuilder>& event_queue,
               std::atomic<bool>& running,
               std::counting_semaphore<>& sem)
    : _socket(_io_c, asio::ip::udp::endpoint(asio::ip::udp::v4(), s.port))
    , _components_to_create(std::ref(comp_queue))
    , _events_to_transmit(std::ref(event_queue))
    , _running(running)
    , _semaphore(std::ref(sem))
    , _queue_reader([this]() { this->send_comp(); })
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> dis;
  _server_id = dis(gen);
}

void Server::close()
{
  if (_socket.is_open()) {
    _socket.close();
  }
}

Server::~Server()
{
  this->_semaphore.get().release();
  if (this->_queue_reader.joinable()) {
    this->_queue_reader.join();
  }
  _socket.close();
}

void Server::receive_loop()
{
  CircularBuffer<BUFFER_SIZE> recv_buf;
  asio::ip::udp::endpoint sender_endpoint;

  while (_running) {
    try {
      std::error_code ec;
      std::size_t len =
          recv_buf.read_socket(this->_socket, sender_endpoint, ec);
      if (len > 0) {
        NETWORK_LOGGER("server",
                       std::uint8_t(LogLevel::DEBUG),
                       std::format("received buffer, size : {}", len));
      }

      if (ec) {
        if (_running) {
          NETWORK_LOGGER("server",
                         std::uint8_t(LogLevel::ERROR),
                         std::format("Receive error: {}", ec.message()));
        }
        break;
      }

      while (std::optional<ByteArray> p = recv_buf.extract(PROTOCOL_EOF)) {
        NETWORK_LOGGER(
            "server", std::uint8_t(LogLevel::DEBUG), "package extracted");
        this->handle_package(*p, sender_endpoint);
      }
    } catch (std::exception& e) {
      if (_running) {
        NETWORK_LOGGER("server",
                       std::uint8_t(LogLevel::ERROR),
                       std::format("Error in receive loop: {}", e.what()));
      }
      break;
    }
  }

  NETWORK_LOGGER(
      "server", std::uint8_t(LogLevel::INFO), "Server receive loop ended");
}

void Server::handle_package(ByteArray const& package,
                            const asio::ip::udp::endpoint& sender)
{
  std::optional<Package> pkg = parse_package(package);

  if (!pkg) {
    return;
  }
  if (pkg->magic != MAGIC_SEQUENCE) {
    NETWORK_LOGGER("server",
                   std::uint8_t(LogLevel::DEBUG),
                   "Invalid magic sequence, ignoring.");
    return;
  }
  ClientState state = ClientState::CHALLENGING;
  this->_client_mutex.lock();
  try {
    state = this->find_client_by_endpoint(sender).state;
  } catch (ClientNotFound const&) {
  }
  this->_client_mutex.unlock();
  if (state == ClientState::CONNECTED) {
    auto const& parsed = parse_connected_package(pkg->real_package);
    if (!parsed) {
      return;
    }
    this->handle_connected_packet(parsed.value(), sender);
  } else {
    auto const& parsed = parse_connectionless_package(pkg->real_package);
    if (!parsed) {
      return;
    }
    this->handle_connectionless_packet(parsed.value(), sender);
  }
}

void Server::send(ByteArray const& response,
                  const asio::ip::udp::endpoint& endpoint)
{
  ByteArray pkg = MAGIC_SEQUENCE + response + PROTOCOL_EOF;

  try {
      _socket.send_to(asio::buffer(pkg), endpoint);
  } catch (asio::system_error const&) {
      this->remove_client_by_endpoint(endpoint);
  }

  NETWORK_LOGGER("server",
                 std::uint8_t(LogLevel::DEBUG),
                 std::format("Sent package of size: {}", pkg.size()));
}

void Server::send_connected(ByteArray const& response,
                            const asio::ip::udp::endpoint& endpoint)
{
  ByteArray pkg = type_to_byte(this->_current_index_sequence)
      + type_to_byte<std::uint32_t>(0) + type_to_byte<bool>(true) + response;

  this->_current_index_sequence += 1;
  this->send(pkg, endpoint);
}
