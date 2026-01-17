#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "network/server/BaseServer.hpp"

#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "ServerLaunch.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "network/server/Server.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/events/CleanupEvent.hpp"
#include "plugin/events/CreateEntity.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/NetworkEvents.hpp"
#include "plugin/events/ShutdownEvent.hpp"

BaseServer::BaseServer(std::string const& name,
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

  SUBSCRIBE_EVENT(ServerLaunching, {
    _running = true;

    this->_server_class.emplace(event,
                                _components_to_update,
                                _event_queue_to_client,
                                _event_queue,
                                _running);
    LOGGER("server",
           LogLevel::INFO,
           std::format("Server started on port {}", event.port));

    this->_actual_server = std::thread(&BaseServer::launch_server, this, event);
  })

  SUBSCRIBE_EVENT(ShutdownEvent, {
    _running = false;
    LOGGER("server",
           LogLevel::INFO,
           std::format("Shutdown requested: {}", event.reason));
    // server.close();
  })

  SUBSCRIBE_EVENT(CleanupEvent, {
    _running = false;
    LOGGER("server", LogLevel::DEBUG, "Cleanup requested");
    // server.close();
  })

  SUBSCRIBE_EVENT_PRIORITY(
      NewConnection,
      {
        std::cout << "NEW CONNECTION\n";
        this->_event_manager.get().emit<EventBuilderId>(
            event.client, "NewConnection", event.to_bytes());
      },
      10)

  SUBSCRIBE_EVENT(ComponentBuilder, {
    this->_event_manager.get().emit<ComponentBuilderId>(std::nullopt, event);
  })

  SUBSCRIBE_EVENT(ComponentBuilderId,
                  { this->_components_to_update.push(event); })

  SUBSCRIBE_EVENT(EventBuilderId, { this->_event_queue_to_client.push(event); })

  SUBSCRIBE_EVENT_PRIORITY(
      DisconnectClient,
      {
        if (!this->_server_class) {
          return false;
        }
        this->_server_class->disconnect_client(event.client);
      },
      2)

  this->_registry.get().add_system(
      [this](Registry& /*r*/)
      {
        if (!this->_server_class) {
          return;
        }

        for (auto const& it : this->_server_class->watch_disconected_clients())
        {
          this->_event_manager.get().emit<DisconnectClient>(it);
        }
      });

  this->_registry.get().add_system(
      [this](Registry& /*r*/)
      {
        auto events = this->_event_queue.flush();
        for (auto& evt : events) {
          this->_event_manager.get().emit(evt.event_id, evt.data);
        }
      });

  SUBSCRIBE_EVENT(StateTransfer, {
    std::vector<ComponentState> s = this->_registry.get().get_state();

    for (auto const& it : s) {
      for (auto const& i : it.comps) {
        this->_event_manager.get().emit<ComponentBuilderId>(
            event.client_id, ComponentBuilder(i.first, it.id, i.second));
      }
    }
  })

  SUBSCRIBE_EVENT(LoadEntityTemplate, {
    this->_loader.get().load_entity_template(event.template_name,
                                             event.aditionals);
  })

  SUBSCRIBE_EVENT(CreateEntity, {
    Registry::Entity entity = this->_registry.get().spawn_entity();
    for (auto const& [id, comp] : event.additionals) {
      init_component(
          this->_registry.get(), this->_event_manager.get(), entity, id, comp);
    }
  })

  SUBSCRIBE_EVENT(DeleteEntity, {
    this->_registry.get().kill_entity(event.entity);
    this->_event_manager.get().emit<EventBuilderId>(
        std::nullopt,
        "DeleteClientEntity",
        DeleteClientEntity(event.entity).to_bytes());
  })

  this->setup_http_requests();
}

BaseServer::~BaseServer()
{
  this->unregister_server();
  _running = false;
  if (_server_class) {
    this->_server_class->close();
  }
  if (this->_actual_server.joinable()) {
    this->_actual_server.join();
  }
}

void BaseServer::launch_server(ServerLaunching const& infos)
{
  try {
    this->_port = static_cast<int>(infos.port);
    this->_server_class->receive_loop();
  } catch (std::exception& e) {
    LOGGER("server",
           LogLevel::ERR,
           std::format("Failed to start server: {}", e.what()));
  }
}

int BaseServer::get_user_by_client(std::size_t client_id)
{
  if (!this->_server_class) {
    return -1;
  }
  return this->_server_class->get_user_by_client(client_id);
}

int BaseServer::get_client_by_user(int user)
{
  if (!this->_server_class) {
    return -1;
  }
  return this->_server_class->get_client_by_user(user);
}
