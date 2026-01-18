#include <exception>
#include <format>
#include <optional>
#include <thread>

#include "network/client/BaseClient.hpp"

#include "ClientConnection.hpp"
#include "NetworkShared.hpp"
#include "ecs/Registry.hpp"
#include "network/HttpClient.hpp"
#include "network/client/Client.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Byte.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/CleanupEvent.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/LogMacros.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/NetworkEvents.hpp"
#include "plugin/events/ShutdownEvent.hpp"

BaseClient::BaseClient(std::string const& name,
                       std::string game_name,
                       Registry& r,
                       EventManager& em,
                       EntityLoader& l,
                       std::optional<JsonObject> const& config)
    : APlugin(name, r, em, l, {}, {})
    , game_name(std::move(game_name))
{
  try {
    if (!config) {
      throw std::exception();
    }
    this->_http_client.init(
        std::get<std::string>(config->at("http_host").value),
        std::get<int>(config->at("http_port").value));
  } catch (std::exception const&) {
    LOGGER("client",
           LogLevel::WARNING,
           "failed to init http client, using default 0.0.0.0:8080")
    this->_http_client.init("0.0.0.0", 8080);  // NOLINT
  }
  SUBSCRIBE_EVENT(ClientConnection, {
    if (this->_user_id == -1) {
      LOGGER("client", LogLevel::ERR, "client not logged in");
      return false;
    }
    if (!this->_running) {
      _running = true;

      this->_thread = std::thread(&BaseClient::connection_thread, this, event);
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
      return false;
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
            this->_server_created.emplace(new_entity);
          }
          auto true_entity = this->_server_indexes.at_first(server_comp.entity);

          try {
            this->_loader.get().load_byte_component(
                true_entity, server_comp, this->_server_indexes);
          } catch (InvalidPackage const& e) {
            LOGGER("client", LogLevel::ERR, e.what());
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
    this->_server_created.erase(event.entity);
    this->_registry.get().kill_entity(event.entity);
  })

  SUBSCRIBE_EVENT(ResetClient, {
    std::cout << "RESET EVENT\n";
    for (auto const& entity : this->_server_created) {
      this->_registry.get().kill_entity(entity);
      this->_server_indexes.remove_second(entity);
    }
    this->_server_created.clear();
  })

  SUBSCRIBE_EVENT(Disconnection, {
    this->_running = false;
    this->_event_manager.get().emit<EventBuilder>(
        "DisconnectClient", DisconnectClient(this->_id_in_server).to_bytes());
    if (this->_thread.joinable()) {
      this->_thread.join();
    }
  })

  SUBSCRIBE_EVENT(NetworkStatus, { (void)this; })

  this->setup_http_requests();
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
    client.connect(this->_user_id);
  } catch (std::exception& e) {
    LOGGER("client",
           LogLevel::ERR,
           std::format("Connection failed: {}", e.what()));
    _running = false;
  }
}
