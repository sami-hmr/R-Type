#include <cstring>
#include <format>
#include <stdexcept>
#include <string_view>
#include <thread>

#include "Network.hpp"

#include "Client.hpp"
#include "ClientConnection.hpp"
#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Byte.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Controllable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/events/CleanupEvent.hpp"
#include "plugin/events/CollisionEvent.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/NetworkEvents.hpp"
#include "plugin/events/ShutdownEvent.hpp"

NetworkClient::NetworkClient(Registry& r, EntityLoader& l)
    : APlugin("network_client", r, l, {}, {})
    , _sem(0)
{
  SUBSCRIBE_EVENT(ClientConnection, {
    if (!this->_running) {
      _running = true;

      this->_thread =
          std::thread([this, event]() { this->connection_thread(event); });
    } else {
      LOGGER("client", LogLevel::WARNING, "client already running");
    }
  })

  SUBSCRIBE_EVENT(ShutdownEvent, {
    _running = false;
    LOGGER("client",
           LogLevel::INFO,
           std::format("Shutdown requested: {}", event.reason));
    // _client->close();
  })

  SUBSCRIBE_EVENT(CleanupEvent, {
    _running = false;
    LOGGER("client", LogLevel::DEBUG, "Cleanup requested");
    // _socket->close();
  })

  SUBSCRIBE_EVENT(EventBuilder, {
    if (!this->_running) {
      return;
    }
    EventBuilder true_e(
        event.event_id,
        this->_registry.get().convert_event_entity(
            event.event_id,
            event.data,
            this->_server_indexes.get_second()));  // CLIENT -> SERVER
    this->_event_to_server.lock.lock();
    this->_event_to_server.queue.push(std::move(true_e));
    this->_event_to_server.lock.unlock();
    this->_sem.release();
  })

  SUBSCRIBE_EVENT(PlayerCreation, {
    this->_id_in_server = event.server_id;
    auto zipper = ZipperIndex<Controllable>(this->_registry.get());

    if (zipper.begin() != zipper.end()) {
      std::size_t index = std::get<0>(*zipper.begin());

      this->_server_indexes.insert(event.server_index, index);
    } else {
      LOGGER("client",
             LogLevel::INFO,
             "no bindings detected for client, default applicated (z q s "
             "d, les bindings de thresh tu connais (de la dinde) ? (le joueur de quake "
             "pas le main de baptiste ahah mdr))");

      // std::size_t new_entity = this->_registry.get().spawn_entity();

      // init_component<Controllable>(this->_registry.get(), {
      //   {"Z",
      //       {
      //           {"name",
      //           this->_registry.get().get_event_key<UpdateDirection>()},
      //           {"params", {
      //               {"entity", JsonValue(static_cast<int>(new_entity))},
      //               {"x", JsonValue(static_cast<double>(0))},
      //               {"y", JsonValue(static_cast<double>(1))}
      //           }}
      //       }
      //   }
      // });
      // this->_server_indexes.insert(event.server_index,
      //                              new_entity);  // SERVER -> CLIENT
    }
    this->_registry.get().emit<EventBuilder>(
        "PlayerCreated",
        PlayerCreated(event.server_index, this->_id_in_server).to_bytes());
  })

  SUBSCRIBE_EVENT(WantReady, {
    this->_registry.get().emit<EventBuilder>(
        "PlayerReady", PlayerReady(this->_id_in_server).to_bytes());
  })

  SUBSCRIBE_EVENT(DeleteClientEntity, {
    this->_server_indexes.remove_second(event.entity);
    this->_registry.get().kill_entity(event.entity);
  })


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

          try {
            this->_loader.get().load_byte_component(
                true_entity, server_comp, this->_server_indexes);
            this->_component_queue.queue.pop();
          } catch (InvalidPackage const &e) {
            LOGGER("client", LogLevel::ERROR, e.what());
          }
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
    _running = false;
  }
}

extern "C"
{
void* entry_point(Registry& r, EntityLoader& e)
{
  return new NetworkClient(r, e);
}
}
