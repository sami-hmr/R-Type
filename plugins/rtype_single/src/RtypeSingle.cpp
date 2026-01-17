#include <optional>
#include <string>

#include "../include/RtypeSingle.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Controllable.hpp"
#include "plugin/events/CreateEntity.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/LoadPluginEvent.hpp"
#include "plugin/events/LogMacros.hpp"
#include "plugin/events/LoggerEvent.hpp"
#include "plugin/events/SceneChangeEvent.hpp"
#include "plugin/events/ShutdownEvent.hpp"

RtypeSingle::RtypeSingle(Registry& r,
                         EventManager& em,
                         EntityLoader& l,
                         std::optional<JsonObject> const& /*config*/)
    : APlugin("rtype_single", r, em, l, {}, {})
{
  LOGGER("rtype_single", LogLevel::INFO, "Initializing single-player mode");

  SUBSCRIBE_EVENT(LoadEntityTemplate, {
    LOGGER("rtype_single",
           LogLevel::DEBUG,
           "LoadEntityTemplate: " + event.template_name);
    this->_loader.get().load_entity_template(event.template_name,
                                             event.aditionals);
  })
  SUBSCRIBE_EVENT(CreateEntity, {
    Registry::Entity entity = this->_registry.get().spawn_entity();
  LOGGER("RTypeSingle", LogLevel::WARNING, "entity generated...")
    for (auto const& [id, comp] : event.additionals) {
      init_component(
          this->_registry.get(), this->_event_manager.get(), entity, id, comp);
  LOGGER("RTypeSingle", LogLevel::WARNING, "a component has been done !")
    }
  LOGGER("RTypeSingle", LogLevel::WARNING, "entity components done.")
  })

  SUBSCRIBE_EVENT(PlayerCreated, {
    LOGGER("rtype_single",
           LogLevel::INFO,
           "PlayerCreated button clicked! Starting single-player game...");

    // Search without scene filtering to find ALL Controllable entities
    auto zipper =
        ZipperIndex<Controllable>(this->_registry.get(), SceneState::DISABLED);

    if (zipper.begin() != zipper.end()) {
      std::size_t controllable_entity = std::get<0>(*zipper.begin());

      LOGGER(
          "rtype_single",
          LogLevel::INFO,
          "Found controllable entity: " + std::to_string(controllable_entity));

      JsonObject template_ref;
      template_ref["template"] = JsonValue(std::string("player"));

      LOGGER("rtype_single",
             LogLevel::INFO,
             "Loading player template onto entity "
                 + std::to_string(controllable_entity));
      this->_loader.get().load_components(controllable_entity, template_ref);

      this->_player_entity = controllable_entity;

      LOGGER("rtype_single", LogLevel::INFO, "Transitioning to game scene");
      this->_event_manager.get().emit<SceneChangeEvent>("game", "", true);
    } else {
      LOGGER("rtype_single",
             LogLevel::ERR,
             "No controllable entity found! client_test.json not loaded?");
    }
  })

  SUBSCRIBE_EVENT(DeleteEntity, {
    // Actually delete the entity
    this->_registry.get().kill_entity(event.entity);

    // Check if it's the player and handle game over
    if (this->_player_entity.has_value()
        && this->_player_entity.value() == event.entity)
    {
      LOGGER("rtype_single", LogLevel::INFO, "Player died! Game over.");
      this->_player_entity = std::nullopt;
      this->_event_manager.get().emit<SceneChangeEvent>("death", "", false);
      this->_event_manager.get().emit<ShutdownEvent>("player death", 0);
    }
  })

  SUBSCRIBE_EVENT(ShutdownEvent, {
    if (this->_player_entity.has_value()) {
      this->_registry.get().kill_entity(this->_player_entity.value());
      this->_player_entity = std::nullopt;
    }
  })
}

extern "C"
{
PLUGIN_EXPORT void* entry_point(Registry& r,
                  EventManager& em,
                  EntityLoader& e,
                  std::optional<JsonObject> const& config)
{
  return new RtypeSingle(r, em, e, config);
}
}
