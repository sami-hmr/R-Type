#include <stdexcept>
#include <vector>

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
  ClientInfo& client = this->find_client_by_endpoint(sender);
  client.acknowledge_manager.register_received_package(command);
  std::vector<ConnectedPackage> packages =
      client.acknowledge_manager.extract_available_packages();
  this->_client_mutex.unlock();

  for (auto const& pkg : packages) {
    ByteArray entire = pkg.real_package;

    auto const& parsed = parse_connected_command(entire);
    if (!parsed) {
      continue;
    }
    this->handle_connected_command(parsed.value(), sender);
  }
  this->_client_mutex.lock();
  ClientInfo& same_client = this->find_client_by_endpoint(sender);
  if (packages.size() != 0) {
    same_client.acknowledge_manager.approuve_packages(
        packages[packages.size() - 1].acknowledge);
  }
  for (auto const& it : same_client.acknowledge_manager.get_packages_to_send())
  {
    this->send(it, same_client.endpoint);
  }
  this->_client_mutex.unlock();
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
