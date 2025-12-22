/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Server
*/
#include <atomic>
#include <vector>

#include "network/server/Server.hpp"

#include <asio/system_error.hpp>

#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "plugin/Byte.hpp"

Server::Server(ServerLaunching const& s,
               SharedQueue<ComponentBuilderId>& comp_queue,
               SharedQueue<EventBuilderId>& event_to_client,
               SharedQueue<EventBuilder>& event_to_server,
               std::atomic<bool>& running)
    : _socket(_io_c, asio::ip::udp::endpoint(asio::ip::udp::v4(), s.port))
    , _components_to_create(std::ref(comp_queue))
    , _events_queue_to_client(std::ref(event_to_client))
    , _events_queue_to_serv(std::ref(event_to_server))
    , _running(running)
{
  this->_queue_readers.emplace_back([this]() { this->send_comp(); });
  this->_queue_readers.emplace_back([this]() { this->send_event_to_client(); });
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
  this->_events_queue_to_client.get().release();
  this->_events_queue_to_serv.get().release();
  this->_components_to_create.get().release();
  for (auto& it : this->_queue_readers) {
    if (it.joinable()) {
      it.join();
    }
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
        // NETWORK_LOGGER("server",
        //                std::uint8_t(LogLevel::DEBUG),
        //                std::format("received buffer, size : {}", len));
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
}

void Server::send_connected(ByteArray const& response, ClientInfo& client)
{
  ByteArray pkg = type_to_byte(client.next_send_sequence)
      + type_to_byte<std::size_t>(0) + type_to_byte<bool>(true) + response;

  client.waiting_aprouval.emplace_back(client.next_send_sequence, pkg);

  client.next_send_sequence += 1;
  this->send(pkg, client.endpoint);
}
