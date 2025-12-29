#include <algorithm>
#include <chrono>
#include <vector>

#include "NetworkCommun.hpp"
#include "network/server/Server.hpp"

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
  throw ClientNotFound("client not found");
}

ClientInfo& Server::find_client_by_id(std::size_t id)
{
  for (auto& client : _clients) {
    if (client.state != ClientState::DISCONNECTED && client.client_id == id) {
      return client;
    }
  }
  throw ClientNotFound("client not found");
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

  NETWORK_LOGGER(
      "server", info, std::format("client {} disconected", client_id));
}

std::vector<std::size_t> Server::watch_disconected_clients()
{
  std::size_t now = std::chrono::steady_clock::now().time_since_epoch().count();
  std::vector<std::size_t> result;

  this->_client_mutex.lock();
  for (auto const& client : this->_clients) {
    if (now > (client.last_ping + client_disconect_timout)) {
      NETWORK_LOGGER(
          "server",
          info,
          std::format("client {} timeouted", client.client_id));
      result.push_back(client.client_id);
    }
  }
  this->_client_mutex.unlock();
  return result;
}
