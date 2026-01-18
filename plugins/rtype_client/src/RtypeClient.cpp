#include <cstring>
#include <format>
#include <optional>

#include "../plugins/rtype_client/include/RtypeClient.hpp"

#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "network/client/BaseClient.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Controllable.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/components/Text.hpp"
#include "plugin/events/HttpEvents.hpp"
#include "plugin/events/InventoryEvents.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/NetworkEvents.hpp"
#include "plugin/events/SceneChangeEvent.hpp"

RtypeClient::RtypeClient(Registry& r, EventManager& em, EntityLoader& l,  std::optional<JsonObject> const &config)
    : BaseClient("rtype_client", "r-type", r, em, l, config)
{
  SUBSCRIBE_EVENT(PlayerCreation, {
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
      //           }}"game"
      //       }
      //   }
      // });
      // this->_server_indexes.insert(event.server_index,
      //                              new_entity);  // SERVER -> CLIENT
    }

    this->_event_manager.get().emit<EventBuilder>(
        "PlayerCreated",
        PlayerCreated(event.server_index, this->_id_in_server).to_bytes());
  })

  SUBSCRIBE_EVENT(WantReady, {
    this->_event_manager.get().emit<EventBuilder>(
        "PlayerReady", PlayerReady(this->_id_in_server).to_bytes());
  })

  SUBSCRIBE_EVENT(Disconnection, {
    this->_event_manager.get().emit<DisableSceneEvent>("connecting_card");
    this->alert("Disconected");
  })

  SUBSCRIBE_EVENT(Logout, {
    this->_event_manager.get().emit<SceneChangeEvent>("login", "", true);
    this->_event_manager.get().emit<SceneChangeEvent>("connection_background", "", false);
    this->_event_manager.get().emit<SceneChangeEvent>("game", "", false);
  })
  this->handle_http();
}

void RtypeClient::alert(std::string const& message)
{
  this->_event_manager.get().emit<SceneChangeEvent>(
      "alert", "connected", false, true);
  for (auto [e, text, scene, team] :
       ZipperIndex<Text, Scene, Team>(this->_registry.get()))
  {
    if (scene.scene_name != "alert" || team.name != "message") {
      continue;
    }
    text.text = message;
  }
}

extern "C"
{
PLUGIN_EXPORT void* entry_point(Registry& r, EventManager& em, EntityLoader& e, std::optional<JsonObject> const &config)
{
  return new RtypeClient(r, em, e, config);
}
}
