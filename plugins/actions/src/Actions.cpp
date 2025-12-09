#include <any>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>

#include "Actions.hpp"

#include "Json/JsonParser.hpp"
#include "TwoWayMap.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/ActionTrigger.hpp"
#include "plugin/events/ActionEvents.hpp"
#include "plugin/events/Events.hpp"
#include "plugin/events/IoEvents.hpp"

Actions::Actions(Registry& r, EntityLoader& l)
    : APlugin(r,
              l,
              {},
              {{
                  COMP_INIT(ActionTrigger, ActionTrigger, init_action_trigger),
              }})
{
  _registry.get().register_component<ActionTrigger>();

  _registry.get().add_system<>(
      [](Registry& r)
      {
        auto now = std::chrono::steady_clock::now();
        auto delta = r.clock().delta_seconds();
        r.emit<TimerTickEvent>(delta, now);
      },
      5);

  this->_registry.get().on<KeyPressedEvent>(
      "KeyPressedEvent",
      [this](KeyPressedEvent const& evt)
      {
        auto& actions = this->_registry.get().get_components<ActionTrigger>();

        for (auto&& [entity, action] : ZipperIndex(actions)) {
          if (action.event_trigger.first != "KeyPressed") {
            continue;
          }
          std::string key = std::get<std::string>(
              action.event_trigger.second.at("key").value);
          if (std::any_cast<KeyPressedEvent>(evt).key_pressed.contains(
                  KEY_MAPPING.at_first(key)))
          {
            for (auto& i : action.event_to_emit) {
              this->_registry.get().emit(i.first, i.second);
            }
          }
        }
      });
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
  this->_registry.get().emplace_component<ActionTrigger>(
      entity, std::make_pair(type, params), to_emit_map);
}

extern "C"
{
void* entry_point(Registry& r, EntityLoader& e)
{
  return new Actions(r, e);
}
}
