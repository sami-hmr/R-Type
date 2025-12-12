#include <cstring>
#include <format>
#include <stdexcept>
#include <string_view>
#include <thread>

#include "Network.hpp"

#include "Client.hpp"
#include "NetworkShared.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Controllable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/events/CleanupEvent.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/ShutdownEvent.hpp"

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
        EventBuilder true_e(
            c.event_id,
            this->_registry.get().convert_event_entity(
                c.event_id,
                c.data,
                this->_server_indexes.get_second()));  // CLIENT -> SERVER
        this->_event_to_server.lock.lock();
        this->_event_to_server.queue.push(std::move(true_e));
        this->_event_to_server.lock.unlock();
        this->_sem.release();
      });

  this->_registry.get().on<PlayerCreation>(
      "PlayerCreation",
      [this](PlayerCreation const& server)
      {
        auto zipper =
            ZipperIndex(this->_registry.get().get_components<Controllable>());

        if (zipper.begin() != zipper.end()) {
          std::size_t index = std::get<0>(*zipper.begin());

          this->_server_indexes.insert(server.server_index, index);

        } else {
          LOGGER("client",
                 LogLevel::INFO,
                 "no bindings detected for client, default applicated (z q s "
                 "d, les bindings de thresh tu connais (de la dinde) ? (le joueur de quake "
                 "pas le main de baptiste ahah mdr))");

          std::size_t new_entity = this->_registry.get().spawn_entity();

          this->_registry.get().emplace_component<Controllable>(
              new_entity, 'Z', 'S', 'Q', 'D');
          this->_server_indexes.insert(server.server_index, new_entity); // SERVER -> CLIENT
        }
        this->_registry.get().emit<EventBuilder>(
            "PlayerCreated", PlayerCreated(server.server_index).to_bytes());
      });

  this->_registry.get().add_system<>(
      [this](Registry& r)
      {
        if (!this->_running) {
          return;
        }
        this->_component_queue.lock.lock();
        while (!this->_component_queue.queue.empty()) {
          auto& server_comp = this->_component_queue.queue.front();

          if (!this->_server_indexes.contains_first(server_comp.entity)) {
            auto new_entity = r.spawn_entity();
            this->_server_indexes.insert(server_comp.entity, new_entity);
          }
          auto true_entity = this->_server_indexes.at_first(server_comp.entity);

          r.emplace_component(true_entity,
                              server_comp.id,
                              this->_registry.get().convert_comp_entity(
                                  server_comp.id,
                                  server_comp.data,
                                  this->_server_indexes.get_first()));

          this->_component_queue.queue.pop();
        }
        this->_component_queue.lock.unlock();
      });

  this->_registry.get().add_system<>(
      [this](Registry& r)
      {
        this->_event_from_server.lock.lock();
        while (!this->_event_from_server.queue.empty()) {
          auto& e = this->_event_from_server.queue.front();
          r.emit(e.event_id,
                 this->_registry.get().convert_event_entity(
                     e.event_id,
                     e.data,
                     this->_server_indexes.get_first()));  // SERVER -> CLIENT
          this->_event_from_server.queue.pop();
        }
        this->_event_from_server.lock.unlock();
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
    Client client(c,
                  _component_queue,
                  _event_to_server,
                  _event_from_server,
                  _running,
                  _sem);
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
