#include <algorithm>

#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "ServerCommands.hpp"
#include "network/PacketCompresser.hpp"
#include "network/client/Client.hpp"
#include "plugin/Byte.hpp"
#include "plugin/events/NetworkEvents.hpp"

const std::unordered_map<std::uint8_t, void (Client::*)(ByteArray const&)>
    Client::connected_table = {{SENDCOMP, &Client::handle_component_update},
                               {SENDEVENT, &Client::handle_event_creation},
                               {FFGONEXT, &Client::reset_acknowledge}};

void Client::handle_connected_package(ConnectedPackage const& package)
{
  if (package.prioritary) {
    this->compute_connected_package(package);
    return;
  }
  this->_acknowledge_mutex.lock();
  this->_acknowledge_manager.register_received_package(package);

  auto const& packages =
      this->_acknowledge_manager.extract_available_packages();
  for (auto const& pkg : packages) {
    this->compute_connected_package(pkg);
  }
  if (packages.size() != 0) {
    this->_acknowledge_manager.approuve_packages(
        packages[packages.size() - 1].acknowledge);
  }
  this->_acknowledge_mutex.unlock();
  // for (auto const &to_send :
  // this->_acknowledge_manager.get_packages_to_send()) {
  //     this->send(to_send);
  // }
}

void Client::compute_connected_package(ConnectedPackage const& package)
{
  this->_receive_frag_buffer += package.real_package;
  if (!package.end_of_content) {
    return;
  }
  auto const& parsed = parse_connected_command(
      PacketCompresser::uncompress_packet(this->_receive_frag_buffer));
  this->_receive_frag_buffer.clear();
  if (!parsed) {
    return;
  }
  this->handle_connected_command(parsed.value());
}

void Client::handle_connected_command(ConnectedCommand const& command)
{
  try {
    //   std::cout << (int)command.opcode << std::endl;
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

void Client::send_connected(ByteArray const& response, bool prioritary)
{
  ByteArray compressed = PacketCompresser::compress_packet(response);
  std::vector<ByteArray> const& packages =
      compressed / get_package_division(compressed.size());
  for (std::size_t i = 0; i < packages.size(); i++) {
    ConnectedPackage pkg(this->_index_sequence,
                         this->_acknowledge_manager.get_acknowledge(),
                         (i + 1) == packages.size(),
                         prioritary,
                         packages[i]);

    this->_acknowledge_manager.register_sent_package(pkg);
    this->_index_sequence += 1;
    this->send(pkg.to_bytes());
  }
}

void Client::handle_hearthbeat(ByteArray const& pkg)
{
  auto now = std::chrono::steady_clock::now().time_since_epoch().count();
  auto parsed = parse_hearthbeat_cmd(pkg);

  if (!parsed) {
    return;
  }
  auto const& packages_to_send =
      this->_acknowledge_manager.get_packages_to_send(parsed->lost_packages);
  for (auto const& it : packages_to_send) {
    this->send(it);
  }
  this->_latency_mutex.lock();
  this->_latencies.push_back(now - parsed->send_timestamp);
  this->_latency_mutex.unlock();
}

void Client::reset_acknowledge(ByteArray const& package)
{
  auto parsed = parse_reset_cmd(package);

  if (!parsed) {
    return;
  }
  std::cout << "RESET" << std::endl;
  this->transmit_event(EventBuilder("ResetClient", package));
  this->_acknowledge_mutex.lock();
  this->_acknowledge_manager.reset(parsed->sequence);
  this->_acknowledge_mutex.unlock();
}
