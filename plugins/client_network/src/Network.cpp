#include <cstring>
#include <format>
#include <stdexcept>
#include <thread>

#include "Network.hpp"

#include "Client.hpp"
#include "NetworkShared.hpp"
#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Events.hpp"

NetworkClient::NetworkClient(Registery& r, EntityLoader& l)
    : APlugin(r, l, {}, {})
    , _sem(0)
{
  this->_registery.get().on<ClientConnection>(
      [this](ClientConnection const& c)
      {
        this->_thread =
            std::thread([this, c]() { this->connection_thread(c); });
      });

  this->_registery.get().on<ShutdownEvent>(
      [this](ShutdownEvent const& event)
      {
        _running = false;
        LOGGER("client",
               LogLevel::INFO,
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

  this->_registery.get().on<EventBuilder>(
      [this](EventBuilder c)
      {
        if (!this->_running) {
          return;
        }
        this->_event_queue.lock.lock();
        this->_event_queue.queue.push(std::move(c));
        this->_event_queue.lock.unlock();
        this->_sem.release();
      });

  this->_registery.get().add_system<>(
      [this](Registery& r)
      {
        if (!this->_running) {
          return;
        }
        this->_component_queue.lock.lock();
        while (!this->_component_queue.queue.empty()) {
          auto& e = this->_component_queue.queue.front();
          if (!this->_server_indexes.contains(e.entity)) {
              auto new_entity = r.spawn_entity();
              this->_server_indexes.insert_or_assign(e.entity, new_entity);
          }
          auto true_entity = this->_server_indexes.at(e.entity);
          r.emplace_component(true_entity, e.id, e.data);
          this->_component_queue.queue.pop();
        }
        this->_component_queue.lock.unlock();
      });

  this->_registery.get().add_system<>(
      [this](Registery& r)
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

  this->_thread.join();
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
void* entry_point(Registery& r, EntityLoader& e)
{
  return new NetworkClient(r, e);
}
}
