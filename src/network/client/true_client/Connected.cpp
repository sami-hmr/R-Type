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
  this->_awaiting_packages[package.sequence_number] = package;

  for (auto const& [sequence, pkg] : this->_awaiting_packages) {
    if (sequence != (this->_last_interpreted_sequence + 1)) {
      break;
    }
    //std::cout << sequence << std::endl;
    this->_last_interpreted_sequence += 1;
    this->compute_connected_package(pkg);
  }
  this->_awaiting_packages.erase(
      this->_awaiting_packages.begin(),
      this->_awaiting_packages.upper_bound(this->_last_interpreted_sequence));
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
  ByteArray pkg = type_to_byte<std::size_t>(0)
      + type_to_byte<std::size_t>(this->_last_interpreted_sequence)
      + type_to_byte<bool>(true) + response;

  this->send(pkg);
}
