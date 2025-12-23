#include <iostream>
#include <vector>

#include "network/server/BaseServer.hpp"

#include "NetworkShared.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "network/server/Server.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/events/CleanupEvent.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/NetworkEvents.hpp"
#include "plugin/events/ShutdownEvent.hpp"

BaseServer::BaseServer(std::string const& name,
                       Registry& r,
                       EventManager& em,
                       EntityLoader& l)
    : APlugin(name, r, em, l, {}, {})
{
  SUBSCRIBE_EVENT(ServerLaunching, {
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
        this->_heathbeat[event.client] =
            this->_registry.get().clock().millisecond_now();
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

  SUBSCRIBE_EVENT(HearthBeat, {
    this->_heathbeat[event.client] =
        this->_registry.get().clock().millisecond_now();
  });

  SUBSCRIBE_EVENT_PRIORITY(DisconnectClient, {
    if (!this->_server_class) {
      return;
    }

    std::cout << "DISCONNECT\n";
    this->_server_class->disconnect_client(event.client);
  }, 2)

  this->_registry.get().add_system(
      [this](Registry& /*r*/)
      {
        std::size_t milliseconds =
            this->_registry.get().clock().millisecond_now();

        std::vector<std::size_t> clients_to_remove;
        for (auto const& [client, delta] : this->_heathbeat) {
          if ((milliseconds - delta) > (1000 * 3 /* 3 seconds */)) {
            clients_to_remove.push_back(client);
          }
        }
        for (auto const &client : clients_to_remove) {
            this->_event_manager.get().emit<DisconnectClient>(client);
            this->_heathbeat.erase(client);
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
    auto const& entity = this->_loader.get().load_entity(
        JsonObject({{"template", JsonValue(event.template_name)}}));

    if (!entity) {
      LOGGER("load entity template",
             LogLevel::ERROR,
             "failed to load entity template " + event.template_name);
    }
    for (auto const& [id, comp] : event.aditionals) {
      init_component(
          this->_registry.get(), this->_event_manager.get(), *entity, id, comp);
    }
  })

  SUBSCRIBE_EVENT(DeleteEntity, {
    this->_registry.get().kill_entity(event.entity);
    this->_event_manager.get().emit<EventBuilderId>(
        std::nullopt,
        "DeleteClientEntity",
        DeleteClientEntity(event.entity).to_bytes());
  })
}

BaseServer::~BaseServer()
{
  _running = false;
  if (this->_actual_server.joinable()) {
    this->_actual_server.join();
  }
}

void BaseServer::launch_server(ServerLaunching const& s)
{
  try {
    _running = true;

    this->_server_class.emplace(s,
                                _components_to_update,
                                _event_queue_to_client,
                                _event_queue,
                                _running);
    LOGGER("server",
           LogLevel::INFO,
           std::format("Server started on port {}", s.port));

    this->_server_class->receive_loop();
  } catch (std::exception& e) {
    LOGGER("server",
           LogLevel::ERROR,
           std::format("Failed to start server: {}", e.what()));
  }
}
