#include <cstring>
#include <format>
#include <stdexcept>
#include <thread>

#include "Network.hpp"

#include "Client.hpp"
#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Events.hpp"

NetworkClient::NetworkClient(Registery& r, EntityLoader& l)
    : APlugin(r, l, {}, {})
{
  this->_registery.get().on<ClientConnection>(
      [this](ClientConnection const& c)
      {
        this->_threads.emplace_back([this, c]()
                                    { this->connection_thread(c); });
      });

  this->_registery.get().on<ShutdownEvent>(
      [this](ShutdownEvent const& event)
      {
        _running = false;
        LOGGER("client", LogLevel::INFO,
         std::format("Shutdown requested: {}", event.reason));
          // _client->close();
      });

  this->_registery.get().on<CleanupEvent>(
      [this](CleanupEvent const&)
      {
        _running = false;
        LOGGER("client", LogLevel::DEBUG, "Cleanup requested");
        // _socket->close();
      });
}

NetworkClient::~NetworkClient()
{
  _running = false;
  for (auto& t : this->_threads) {
    t.join();
  }
}

void NetworkClient::connection_thread(ClientConnection const& c)
{
  try {
    Client client(c, _component_queue, _running);
    client.connect();
  } catch (std::exception& e) {
    LOGGER("client",
           LogLevel::ERROR,
           std::format("Connection failed: {}", e.what()));
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new NetworkClient(r, e);
}
}
