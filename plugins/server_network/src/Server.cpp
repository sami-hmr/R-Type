/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Server
*/
#include <atomic>
#include <queue>

#include "Server.hpp"

#include "Network.hpp"
#include "NetworkShared.hpp"
// #include "plugin/events/Events.hpp"

const std::unordered_map<std::uint8_t,
                         void (Server::*)(ByteArray const&,
                                          const asio::ip::udp::endpoint&)>
    Server::command_table = {
        {GETINFO, &Server::handle_getinfo},
        {GETSTATUS, &Server::handle_getstatus},
        {GETCHALLENGE, &Server::handle_getchallenge},
        {CONNECT, &Server::handle_connect},
};

Server::Server(ServerLaunching const& s,
               SharedQueue<ComponentBuilder> &shared_queue,
               std::atomic<bool>& running)
    : _socket(_io_c, asio::ip::udp::endpoint(asio::ip::udp::v4(), s.port))
    , _shared_queue(std::ref(shared_queue))
    , _running(running)
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
        // for (auto it : *p) {
        //   std::cout << (int)it << std::endl;
        // }
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
  std::optional<Package> pkg = this->parse_package(package);

  if (!pkg) {
    return;
  }
  if (pkg->magic != MAGIC_SEQUENCE) {
    NETWORK_LOGGER("server",
                   std::uint8_t(LogLevel::DEBUG),
                   "Invalid magic sequence, ignoring.");
    return;
  }
  auto const& parsed = this->parse_connectionless_package(pkg->real_package);
  // TODO: handle in different function connected and not connected package
  if (!parsed) {
    return;
  }
  handle_connectionless_packet(parsed.value(), sender);
}
