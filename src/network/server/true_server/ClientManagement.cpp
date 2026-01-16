#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>

#include <asio/ip/udp.hpp>

#include "NetworkCommun.hpp"
#include "NetworkShared.hpp"
#include "network/server/Server.hpp"
#include "plugin/Byte.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/NetworkEvents.hpp"

ClientInfo& Server::find_client_by_endpoint(
    const asio::ip::udp::endpoint& endpoint)
{
  for (auto& client : _clients) {
    if (client.state != ClientState::DISCONNECTED
        && client.endpoint == endpoint)
    {
      return client;
    }
  }
  throw ClientNotFound("client not found")
      .with_context(
          "endpoint",
          std::format(
              "{}:{}", endpoint.address().to_string(), endpoint.port()));
}

ClientInfo& Server::find_client_by_id(std::size_t id)
{
  for (auto& client : _clients) {
    if (client.state != ClientState::DISCONNECTED && client.client_id == id) {
      return client;
    }
  }
  throw ClientNotFound("client not found")
      .with_context("client_id", std::to_string(id));
}

ClientInfo& Server::find_client_by_user(int id)
{
  for (auto& client : _clients) {
    if (client.state != ClientState::DISCONNECTED && client.user_id == id) {
      return client;
    }
  }
  throw ClientNotFound("client not found")
      .with_context("client_id", std::to_string(id));
}

void Server::remove_client_by_endpoint(const asio::ip::udp::endpoint& endpoint)
{
  auto it = std::find_if(this->_clients.begin(),
                         this->_clients.end(),
                         [endpoint](ClientInfo const& c)
                         { return c.endpoint == endpoint; });
  if (it != this->_clients.end()) {
    this->_clients.erase(it);
  }
}

void Server::remove_client_by_id(std::size_t client_id)
{
  auto it = std::find_if(this->_clients.begin(),
                         this->_clients.end(),
                         [client_id](ClientInfo const& c)
                         { return c.client_id == client_id; });
  if (it != this->_clients.end()) {
    this->_clients.erase(it);
  }
}

void Server::disconnect_client(std::size_t client_id)
{
  this->_client_mutex.lock();
  this->remove_client_by_id(client_id);
  this->_client_mutex.unlock();

  LOGGER_EVTLESS(LogLevel::INFO,
                 "server",
                 std::format("client {} disconected", client_id));
}

std::vector<std::size_t> Server::watch_disconected_clients()
{
  std::size_t now = std::chrono::steady_clock::now().time_since_epoch().count();
  std::vector<std::size_t> result;

  this->_client_mutex.lock();
  for (auto const& client : this->_clients) {
    if (now > (client.last_ping + client_disconect_timout)) {
      LOGGER_EVTLESS(LogLevel::INFO,
                     "server",
                     std::format("client {} timeouted", client.client_id));
      result.push_back(client.client_id);
    }
  }
  this->_client_mutex.unlock();
  return result;
}

void Server::reset_client_by_endpoint(asio::ip::udp::endpoint const& client)
{
  ClientInfo& c = this->find_client_by_endpoint(client);
  std::size_t const& now =
      std::chrono::steady_clock::now().time_since_epoch().count();

  if (now - c.last_reset > reset_delta) {
    c.reset_count = 0;
  }
  c.last_reset = now;
  c.reset_count++;
  if (c.reset_count >= reset_max_count) {
    LOGGER_EVTLESS(
        LogLevel::ERR,
        "server",
        std::format("Reset count too hight for client {}. Disconnecting",
                    c.client_id));
    this->transmit_event_to_server(EventBuilder(
        "DisconnectClient", DisconnectClient(c.client_id).to_bytes()));
  } else {
    std::cout << "RESET\n";
    this->send_connected(
        type_to_byte<Byte>(FFGONEXT) + type_to_byte(c.next_send_sequence),
        c,
        true);
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    c.acknowledge_manager.reset();
    this->transmit_event_to_server(
        EventBuilder("StateTransfer", StateTransfer(c.client_id).to_bytes()));
  }
}

int Server::get_user_by_client(std::size_t id)
{
  this->_client_mutex.lock();
  int user = -1;
  try {
    auto& client = this->find_client_by_id(id);
    this->_client_mutex.unlock();
    user = client.user_id;
  } catch (ClientNotFound const&) {  // NOLINT
  }
  this->_client_mutex.unlock();
  return user;
}

int Server::get_client_by_user(int id)
{
  this->_client_mutex.lock();
  int client_id = -1;
  try {
    auto& client = this->find_client_by_user(id);
    this->_client_mutex.unlock();
    client_id = client.client_id;
  } catch (ClientNotFound const&) {  // NOLINT
  }
  this->_client_mutex.unlock();
  return client_id;
}
