#include <cstring>
#include <format>
#include <stdexcept>
#include <thread>

#include "Network.hpp"

#include "Client.hpp"
#include "NetworkShared.hpp"
#include "ecs/Registry.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Events.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/Shutdown.hpp"

NetworkClient::NetworkClient(Registry& r, EntityLoader& l)
    : APlugin(r, l, {}, {})
    , _sem(0)
{
  this->_registry.get().on<ClientConnection>(
      "ClientConnection",
      [this](ClientConnection const& c)
      {
        if (!this->_running) {
          _running = true;

          this->_thread =
              std::thread([this, c]() { this->connection_thread(c); });
        } else {
          LOGGER("client", LogLevel::WARNING, "client already running");
        }
      });

  this->_registry.get().on<ShutdownEvent>(
      "ShutdownEvent",
      [this](ShutdownEvent const& event)
      {
        _running = false;
        LOGGER("client",
               LogLevel::INFO,
               std::format("Shutdown requested: {}", event.reason));
        // _client->close();
      });

  this->_registry.get().on<CleanupEvent>(
      "CleanupEvent",
      [this](CleanupEvent const&)
      {
        _running = false;
        LOGGER("client", LogLevel::DEBUG, "Cleanup requested");
        // _socket->close();
      });

  this->_registry.get().on<EventBuilder>(
      "EventBuilder",
      [this](EventBuilder const& c)
      {
        if (!this->_running) {
          return;
        }
        EventBuilder true_e(c.event_id,
                            this->_registry.get().convert_event_entity(
                                c.event_id, c.data, this->_server_indexes));
        this->_event_queue.lock.lock();
        this->_event_queue.queue.push(std::move(true_e));
        this->_event_queue.lock.unlock();
        this->_sem.release();
      });

  this->_registry.get().add_system<>(
      [this](Registry& r)
      {
        if (!this->_running) {
          return;
        }
        this->_component_queue.lock.lock();
        while (!this->_component_queue.queue.empty()) {
          auto& e = this->_component_queue.queue.front();
          if (!this->_server_indexes.contains_first(e.entity)) {
            auto new_entity = r.spawn_entity();
            this->_server_indexes.insert(e.entity, new_entity);
          }
          auto true_entity = this->_server_indexes.at_first(e.entity);
          r.emplace_component(true_entity, e.id, e.data);
          this->_component_queue.queue.pop();
        }
        this->_component_queue.lock.unlock();
      });

  this->_registry.get().add_system<>(
      [this](Registry& r)
      {
        this->_exec_event_queue.lock.lock();
        while (!this->_exec_event_queue.queue.empty()) {
          auto& e = this->_exec_event_queue.queue.front();
          // r.emit(e.event_id, e.data);
          this->_exec_event_queue.queue.pop();
        }
        this->_exec_event_queue.lock.unlock();
      });
}

NetworkClient::~NetworkClient()
{
  _running = false;

  if (this->_thread.joinable()) {
    this->_thread.join();
  }
}

void NetworkClient::connection_thread(ClientConnection const& c)
{
  try {
    Client client(
        c, _component_queue, _event_queue, _exec_event_queue, _running, _sem);
    client.connect();
  } catch (std::exception& e) {
    LOGGER("client",
           LogLevel::ERROR,
           std::format("Connection failed: {}", e.what()));
  }
}

extern "C"
{
void* entry_point(Registry& r, EntityLoader& e)
{
  return new NetworkClient(r, e);
}
}
