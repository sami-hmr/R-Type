#include <algorithm>
#include <chrono>
#include <format>
#include <functional>
#include <optional>
#include <thread>

#include "Network.hpp"

#include <unistd.h>

#include "NetworkShared.hpp"
#include "Server.hpp"
#include "ServerLaunch.hpp"
#include "ecs/ComponentState.hpp"
#include "ecs/EmitEvent.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/BasicWeapon.hpp"
#include "plugin/components/Collidable.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Health.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/components/Velocity.hpp"
#include "plugin/events/CleanupEvent.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/NetworkEvents.hpp"
#include "plugin/events/SceneChangeEvent.hpp"
#include "plugin/events/ShutdownEvent.hpp"

NetworkServer::NetworkServer(Registry& r, EntityLoader& l)
    : APlugin("network_server", r, l, {}, {})
    , _comp_semaphore(0)
    , _semaphore_event_to_server(0)

    , _event_semaphore(0)
{
  SUBSCRIBE_EVENT(ServerLaunching, {
    this->_thread = std::thread(&NetworkServer::launch_server, this, event);
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

  SUBSCRIBE_EVENT(ComponentBuilder, {
    this->_registry.get().emit<ComponentBuilderId>(std::nullopt, event);
  })

  SUBSCRIBE_EVENT(ComponentBuilderId, {
    this->_components_to_update.lock.lock();
    this->_components_to_update.queue.push(event);
    this->_components_to_update.lock.unlock();
    this->_comp_semaphore.release();
  })

  SUBSCRIBE_EVENT(EventBuilderId, {
    this->_event_queue_to_client.lock.lock();
    this->_event_queue_to_client.queue.push(event);
    this->_event_queue_to_client.lock.unlock();
    this->_event_semaphore.release();
  })

  this->_registry.get().add_system<>(
      [this](Registry& r)
      {
        this->_event_queue.lock.lock();
        while (!this->_event_queue.queue.empty()) {
          auto& e = this->_event_queue.queue.front();
          r.emit(e.event_id, e.data);
          this->_event_queue.queue.pop();
        }
        this->_event_queue.lock.unlock();
      });

  SUBSCRIBE_EVENT(EntityCreation, {
    std::size_t entity = this->_registry.get().spawn_entity();

    this->_registry.get().emit<EventBuilderId>(
        event.client,
        "PlayerCreation",
        PlayerCreation(entity, event.client).to_bytes());

    this->_player_ready[event.client] = false;
  })

  SUBSCRIBE_EVENT(PlayerCreated, {
    this->_registry.get().emit<StateTransfer>(event.client_id);

    this->_registry.get().emit<EventBuilderId>(
        event.client_id,
        "SceneChangeEvent",
        SceneChangeEvent("loby", "", true).to_bytes());

    init_component(
        this->_registry.get(), event.server_index, Position(0, 0, 2));
    init_component(this->_registry.get(), event.server_index, Drawable());
    init_component(
        this->_registry.get(), event.server_index, Velocity(0.01, 0.01, 0, 0));

    init_component(this->_registry.get(),
                   event.server_index,
                   AnimatedSprite({{"idle",
                                    AnimationData("assets/player.png",
                                                  {350., 150.},
                                                  {0., 0.},
                                                  {1., 0.},
                                                  {0.2, 0.2},
                                                  10,
                                                  7,
                                                  0,
                                                  true,
                                                  true)}},
                                  "idle",
                                  "idle"));

    init_component(this->_registry.get(),
                   event.server_index,
                   Collidable(0.02, 0.02, CollisionType::Solid));
    init_component(this->_registry.get(), event.server_index, Health(5, 100));
    init_component(this->_registry.get(), event.server_index, Team("test1"));
    init_component(this->_registry.get(),
                   event.server_index,
                   Scene("game", SceneState::ACTIVE));
    init_component(this->_registry.get(),
                   event.server_index,
                   BasicWeapon("basic_bullet", 6, 3, 2.0, 0.3));
  })

  SUBSCRIBE_EVENT(StateTransfer, {
    std::vector<ComponentState> s = this->_registry.get().get_state();

    for (auto const& it : s) {
      for (auto const& i : it.comps) {
        this->_registry.get().emit<ComponentBuilderId>(
            event.client_id, ComponentBuilder(i.first, it.id, i.second));
      }
    }
  })

  SUBSCRIBE_EVENT(PlayerReady, {
    if (!this->_player_ready.contains(event.client_id)) {
      return;
    }
    this->_player_ready[event.client_id] = true;

    this->_registry.get().emit<EventBuilderId>(
        event.client_id,
        "SceneChangeEvent",
        SceneChangeEvent("ready", "", true).to_bytes());

    if (std::find_if(this->_player_ready.begin(),
                     this->_player_ready.end(),
                     [](auto const& p) { return !p.second; })
        == this->_player_ready.end())
    {
      this->_registry.get().emit<SceneChangeEvent>("game", "", true);
      this->_registry.get().emit<EventBuilderId>(
          std::nullopt,
          "SceneChangeEvent",
          SceneChangeEvent("game", "", true).to_bytes());
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
      init_component(this->_registry.get(), *entity, id, comp);
    }
  })

  SUBSCRIBE_EVENT(DeleteEntity, {
    this->_registry.get().kill_entity(event.entity);
    this->_registry.get().emit<EventBuilderId>(
        std::nullopt,
        "DeleteClientEntity",
        DeleteClientEntity(event.entity).to_bytes());
  })
}

NetworkServer::~NetworkServer()
{
  _running = false;
  if (this->_thread.joinable()) {
    this->_thread.join();
  }
}

void NetworkServer::launch_server(ServerLaunching const& s)
{
  try {
    _running = true;

    Server server(s,
                  _components_to_update,
                  _event_queue_to_client,
                  _event_queue,
                  _running,
                  _comp_semaphore,
                  _event_semaphore,
                  _semaphore_event_to_server);

    LOGGER("server",
           LogLevel::INFO,
           std::format("Server started on port {}", s.port));

    server.receive_loop();
  } catch (std::exception& e) {
    LOGGER("server",
           LogLevel::ERROR,
           std::format("Failed to start server: {}", e.what()));
  }
}

extern "C"
{
void* entry_point(Registry& r, EntityLoader& e)
{
  return new NetworkServer(r, e);
}
}
