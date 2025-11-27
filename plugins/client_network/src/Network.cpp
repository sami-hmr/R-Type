#include <format>
#include "Network.hpp"
#include <asio/connect.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/registered_buffer.hpp>

#include "ClientConnection.hpp"
#include "Events.hpp"
#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"



void NetworkClient::reading_thread(ClientConnection const &con) {
    asio::ip::tcp::endpoint ep(asio::ip::address::from_string(con.host), con.port);
    std::array<char, 128> buf;

    asio::ip::tcp::socket socket(this->_io_c);
    LOGGER("network", LogLevel::DEBUG, "try connect");
    socket.connect(ep);
    LOGGER("network", LogLevel::DEBUG, "succeed connect");

    std::size_t len = socket.read_some(asio::buffer(buf));
    std::cout.write(buf.data(), len);
    std::cout << "\n";
}


extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new NetworkClient(r, e);
}
}
