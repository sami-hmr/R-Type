#include <algorithm>

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
  this->_client_mutex.lock();
  auto it = std::find_if(this->_clients.begin(),
                         this->_clients.end(),
                         [endpoint](ClientInfo const& c)
                         { return c.endpoint == endpoint; });
  if (it != this->_clients.end()) {
    this->_clients.erase(it);
  }
  this->_client_mutex.unlock();
}

void Server::remove_client_by_id(std::size_t client_id)
{
  this->_client_mutex.lock();
  auto it = std::find_if(this->_clients.begin(),
                         this->_clients.end(),
                         [client_id](ClientInfo const& c)
                         { return c.client_id == client_id; });
  if (it != this->_clients.end()) {
    this->_clients.erase(it);
  }
  this->_client_mutex.unlock();
}

void Server::disconnect_client(std::size_t client_id)
{
  this->remove_client_by_id(client_id);

  NETWORK_LOGGER(
      "server", info, std::format("client {} disconected", client_id));
}
