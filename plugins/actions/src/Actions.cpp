#include <any>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>

#include "Actions.hpp"

#include "Json/JsonParser.hpp"
#include "TwoWayMap.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/ActionTrigger.hpp"
#include "plugin/events/ActionEvents.hpp"
#include "plugin/events/IoEvents.hpp"

Actions::Actions(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("actions",
              r,
              em,
              l,
              {},
              {
                  COMP_INIT(ActionTrigger, ActionTrigger, init_action_trigger),
              })
{
  REGISTER_COMPONENT(ActionTrigger)

  _registry.get().add_system<>(
      [this](Registry& r)
      {
        auto now = std::chrono::steady_clock::now();
        auto delta = r.clock().delta_seconds();
        this->_event_manager.get().emit<TimerTickEvent>(delta, now);
      },
      5);

  SUBSCRIBE_EVENT(KeyPressedEvent, {
    std::vector<std::function<void()>> to_emit;

    for (auto&& [entity, action] :
         ZipperIndex<ActionTrigger>(this->_registry.get()))
    {
      if (!this->_registry.get().is_in_main_scene(entity)
          || action.event_trigger.first != "KeyPressed")
      {
        continue;
      }
      std::string key =
          std::get<std::string>(action.event_trigger.second.at("key").value);
      if (std::any_cast<KeyPressedEvent>(event).key_pressed.contains(
              KEY_MAPPING.at_first(key)))
      {
        for (auto& i : action.event_to_emit) {
          to_emit.emplace_back(
              [&]()
              {
                this->_event_manager.get().emit(
                    this->_registry.get(), i.first, i.second);
              });
        }
      }
    }
    for (auto const& it : to_emit) {
      it();
    }
  })
}

void Actions::init_action_trigger(Registry::Entity const& entity,
                                  JsonObject& obj)
{
  JsonObject trigger = std::get<JsonObject>(obj.at("trigger").value);

  std::string type = get_value<ActionTrigger, std::string>(
                         this->_registry.get(), trigger, entity, "type")
                         .value();

  JsonObject params;
  if (trigger.contains("params")) {
    params = get_value<ActionTrigger, JsonObject>(
                 this->_registry.get(), trigger, entity, "params")
                 .value();
  }

  std::vector<std::pair<std::string, JsonObject>> to_emit_map;
  JsonArray to_emit = std::get<JsonArray>(obj.at("to_emit").value);
  for (auto const& i : to_emit) {
    JsonObject obj_array = std::get<JsonObject>(i.value);
    std::string name = get_value<ActionTrigger, std::string>(
                           this->_registry.get(), obj_array, entity, "name")
                           .value();
    JsonObject params;
    if (trigger.contains("params")) {
      params = get_value<ActionTrigger, JsonObject>(
                   this->_registry.get(), obj_array, entity, "params")
                   .value();
    }
    to_emit_map.emplace_back(name, params);
  }

  init_component<ActionTrigger>(this->_registry.get(),
                                this->_event_manager.get(),
                                entity,
                                std::make_pair(type, params),
                                to_emit_map);
}

extern "C"
{
void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new Actions(r, em, e);
}
}
