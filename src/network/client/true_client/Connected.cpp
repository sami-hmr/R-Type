#include <algorithm>

#include "NetworkCommun.hpp"
#include "ServerCommands.hpp"
#include "network/client/Client.hpp"
#include "plugin/Byte.hpp"

const std::unordered_map<std::uint8_t, void (Client::*)(ByteArray const&)>
    Client::connected_table = {{SENDCOMP, &Client::handle_component_update},
                               {SENDEVENT, &Client::handle_event_creation}};

void Client::handle_connected_package(ConnectedPackage const& package)
{
  this->_acknowledge_manager.register_received_package(package);

  auto const &packages = this->_acknowledge_manager.extract_available_packages();
  for (auto const& pkg : packages)
  {
    this->compute_connected_package(pkg);
  }
  if (packages.size() != 0) {
      this->_acknowledge_manager.approuve_packages(packages[packages.size() - 1].acknowledge);
  }
  for (auto const &to_send : this->_acknowledge_manager.get_packages_to_send()) {
      this->send(to_send);
  }
}

void Client::compute_connected_package(ConnectedPackage const& package)
{
  auto const& parsed = parse_connected_command(package.real_package);

  if (!parsed) {
    return;
  }
  this->handle_connected_command(parsed.value());
}

void Client::handle_connected_command(ConnectedCommand const& command)
{
  try {
    (this->*(connected_table.at(command.opcode)))(command.real_package);
  } catch (std::out_of_range const&) {
    NETWORK_LOGGER("client",
                   LogLevel::Waring,
                   std::format("Unknow opcode: '{}'", command.opcode));
  }
}

void Client::handle_component_update(ByteArray const& package)
{
  auto parsed = parse_component_build_cmd(package);

  if (!parsed) {
    return;
  }
  this->transmit_component(std::move(*parsed));
}

void Client::handle_event_creation(ByteArray const& package)
{
  auto parsed = parse_event_build_cmd(package);

  if (!parsed) {
    return;
  }
  this->transmit_event(std::move(*parsed));
}

void Client::send_connected(ByteArray const& response)
{
  ConnectedPackage pkg(this->_index_sequence, this->_acknowledge_manager.get_acknowledge(), true, response);

  this->_acknowledge_manager.register_sent_package(pkg);
  this->_index_sequence += 1;

  this->send(pkg.to_bytes());
}
