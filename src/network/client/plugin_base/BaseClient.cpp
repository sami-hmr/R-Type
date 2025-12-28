#include <chrono>
#include <cstring>
#include <format>
#include <stdexcept>
#include <string_view>
#include <thread>

#include "network/client/BaseClient.hpp"

#include "ClientConnection.hpp"
#include "NetworkShared.hpp"
#include "ecs/Registry.hpp"
#include "network/client/Client.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Byte.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/CleanupEvent.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/ShutdownEvent.hpp"

BaseClient::BaseClient(std::string const& name,
                       Registry& r,
                       EventManager& em,
                       EntityLoader& l)
    : APlugin(name, r, em, l, {}, {})
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

  SUBSCRIBE_EVENT(NewConnection, {
      this->_connected = true;

      this->_id_in_server = event.client;
  })

  SUBSCRIBE_EVENT(EventBuilder, {
    if (!this->_running) {
      return;
    }
    this->_event_to_server.push(EventBuilder(
        event.event_id,
        this->_event_manager.get().convert_event_entity(
            event.event_id,
            event.data,
            this->_server_indexes.get_second())));  // CLIENT -> SERVER
  })

  this->_registry.get().add_system(
      [this](Registry& r)
      {
        if (!this->_running) {
          return;
        }
        auto components = this->_component_queue.flush();
        for (auto& server_comp : components) {
          if (!this->_server_indexes.contains_first(server_comp.entity)) {
            auto new_entity = r.spawn_entity();
            this->_server_indexes.insert(server_comp.entity, new_entity);
          }
          auto true_entity = this->_server_indexes.at_first(server_comp.entity);

          try {
            this->_loader.get().load_byte_component(
                true_entity, server_comp, this->_server_indexes);
          } catch (InvalidPackage const& e) {
            LOGGER("client", LogLevel::ERROR, e.what());
          }
        }
      });

  this->_registry.get().add_system(
      [this](Registry& /*r*/)
      {
        auto events = this->_event_from_server.flush();
        for (auto& e : events) {
          this->_event_manager.get().emit(
              e.event_id,
              this->_event_manager.get().convert_event_entity(
                  e.event_id,
                  e.data,
                  this->_server_indexes.get_first()));  // SERVER -> CLIENT
        }
      });

  SUBSCRIBE_EVENT(DeleteClientEntity, {
    this->_server_indexes.remove_second(event.entity);
    this->_registry.get().kill_entity(event.entity);
  })
}

BaseClient::~BaseClient()
{
  _running = false;
  if (this->_thread.joinable()) {
    this->_thread.join();
  }
}

void BaseClient::connection_thread(ClientConnection const& c)
{
  try {
    Client client(
        c, _component_queue, _event_to_server, _event_from_server, _running);
    client.connect();
  } catch (std::exception& e) {
    LOGGER("client",
           LogLevel::ERROR,
           std::format("Connection failed: {}", e.what()));
    _running = false;
  }
}
