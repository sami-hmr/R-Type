#include <algorithm>
#include <functional>
#include <optional>

#include "RtypeServer.hpp"

#include <unistd.h>

#include "NetworkShared.hpp"
#include "ecs/EmitEvent.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "network/server/BaseServer.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/HttpEvents.hpp"
#include "plugin/events/NetworkEvents.hpp"
#include "plugin/events/SceneChangeEvent.hpp"
#include "plugin/events/ShutdownEvent.hpp"

RtypeServer::RtypeServer(Registry& r, EventManager& em, EntityLoader& l)
    : BaseServer("rtype_server", "r-type", r, em, l)
{
  SUBSCRIBE_EVENT(NewConnection, {
    std::size_t entity = this->_registry.get().spawn_entity();

    this->_users_entities[event.user_id] = entity;
    std::cout << "PLAYER CREATION\n";
    this->_event_manager.get().emit<EventBuilderId>(
        event.client,
        "PlayerCreation",
        PlayerCreation(entity, event.client).to_bytes());

    this->_player_entities[entity] = event.client;
    this->_player_ready[event.client] = false;
  })

  SUBSCRIBE_EVENT(PlayerCreated, {
    auto user_id = this->get_user_by_client(event.client_id);
    this->ask_player_save(user_id);
  })

  SUBSCRIBE_EVENT(SavePlayer, {
    this->save_player(event.user);
  })

  SUBSCRIBE_EVENT(PlayerReady, {
    if (!this->_player_ready.contains(event.client_id)) {
      return;
    }
    this->_player_ready[event.client_id] = true;

    this->_event_manager.get().emit<EventBuilderId>(
        event.client_id,
        "SceneChangeEvent",
        SceneChangeEvent("ready", "", false).to_bytes());

    if (std::find_if(this->_player_ready.begin(),
                     this->_player_ready.end(),
                     [](auto const& p) { return !p.second; })
        == this->_player_ready.end())
    {
      this->_event_manager.get().emit<SceneChangeEvent>("game", "", true);
      this->_event_manager.get().emit<EventBuilderId>(
          std::nullopt,
          "SceneChangeEvent",
          SceneChangeEvent("game", "", true).to_bytes());
    }
  })

  SUBSCRIBE_EVENT(DeleteEntity, {
    if (this->_player_entities.contains(event.entity)) {
      this->_event_manager.get().emit<EventBuilderId>(
          this->_player_entities[event.entity],
          "SceneChangeEvent",
          SceneChangeEvent("death", "", false).to_bytes());
      if (this->_player_entities.erase(event.entity)) {
        if (this->_player_entities.empty()) {
          this->_event_manager.get().emit<EventBuilderId>(
              std::nullopt,
              "ShutdownEvent",
              ShutdownEvent("death of all players...", 0).to_bytes());
          this->_event_manager.get().emit<ShutdownEvent>("game ended", 0);
        }
      }
    }
  })

  SUBSCRIBE_EVENT_PRIORITY(
      DisconnectClient,
      {
        std::vector<Registry::Entity> to_delete;
        for (auto const& [entity, id] : this->_player_entities) {
          if (id == event.client) {
            to_delete.push_back(entity);
          }
        }
        for (auto const& it : to_delete) {
          this->_event_manager.get().emit<DeleteEntity>(it);
        }
      },
      4)
}

extern "C"
{
void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new RtypeServer(r, em, e);
}
}
