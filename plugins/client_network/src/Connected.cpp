#include "Client.hpp"
#include "NetworkCommun.hpp"
#include "ServerCommands.hpp"
#include "plugin/Byte.hpp"

const std::unordered_map<std::uint8_t, void (Client::*)(ByteArray const&)>
    Client::connected_table = {
        {SENDCOMP, &Client::handle_component_update},
        {SENDEVENT, &Client::handle_event_creation}
};

void Client::handle_connected_package(ConnectedPackage const& package) {

    if (!package.end_of_content) {
      this->_waiting_packages[package.sequence_number] += package.real_package;
      return;
    }
    ByteArray entire;
    if (this->_waiting_packages.contains(package.sequence_number)) {
      entire = this->_waiting_packages.at(package.sequence_number) + package.real_package;
    } else {
      entire = package.real_package;
    }
    auto const& parsed = parse_connected_command(entire);

    if (!parsed) {
      return;
    }
    this->handle_connected_command(parsed.value());
}

void Client::handle_connected_command(ConnectedCommand const &command) {
    try {
      (this->*(connected_table.at(command.opcode)))(command.real_package);
    } catch (std::out_of_range const&) {
      NETWORK_LOGGER("client",
                     LogLevel::Waring,
                     std::format("Unknow opcode: '{}'", command.opcode));
    }
}

void Client::handle_component_update(ByteArray const &package) {
    auto parsed = parse_component_build_cmd(package);

    if (!parsed) {
        return;
    }
    this->transmit_component(std::move(*parsed));
}

void Client::handle_event_creation(ByteArray const &package) {
    auto parsed = parse_event_build_cmd(package);

    if (!parsed) {
        return;
    }
    this->transmit_event(std::move(*parsed));
}
