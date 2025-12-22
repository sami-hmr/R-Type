#include <stdexcept>

#include "NetworkCommun.hpp"
#include "PackageFragmentation.hpp"
#include "ServerCommands.hpp"
#include "network/server/Server.hpp"
#include "plugin/Byte.hpp"

const std::unordered_map<std::uint8_t,
                         void (Server::*)(ByteArray const&,
                                          const asio::ip::udp::endpoint&)>
    Server::connected_table = {
        {SENDEVENT, &Server::handle_event_receive},
};

void Server::handle_connected_packet(ConnectedPackage const& command,
                                     const asio::ip::udp::endpoint& sender)
{

  this->_client_mutex.lock();
  this->update_acknowledge(command.acknowledge, this->find_client_by_endpoint(sender));
  this->_client_mutex.unlock();

  ByteArray entire = command.real_package;

  auto const& parsed = parse_connected_command(entire);
  if (!parsed) {
    return;
  }
  this->handle_connected_command(parsed.value(), sender);
}

void Server::handle_connected_command(ConnectedCommand const& command,
                                      const asio::ip::udp::endpoint& sender)
{
  try {
    (this->*(connected_table.at(command.opcode)))(command.real_package, sender);
  } catch (std::out_of_range const&) {
    NETWORK_LOGGER("server",
                   LogLevel::Waring,
                   std::format("Unknow opcode: '{}'", command.opcode));
  }
}

void Server::handle_event_receive(ByteArray const& package,
                                  const asio::ip::udp::endpoint& /*endpoint*/)
{
  auto parsed = parse_event_build_cmd(package);

  if (!parsed) {
    return;
  }
  this->transmit_event_to_server(parsed.value());
}
