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
              [this, i, entity]()
              {
                this->_event_manager.get().emit(
                    this->_registry.get(), i.first, i.second, entity);
              });
        }
      }
    }
    for (auto const& it : to_emit) {
      it();
    }
  })
}

void Actions::init_action_trigger(Ecs::Entity const& entity, JsonObject& obj)
{
  JsonObject trigger = std::get<JsonObject>(obj.at("trigger").value);

  auto type_opt = get_value<ActionTrigger, std::string>(
      this->_registry.get(), trigger, entity, "type");

  if (!type_opt.has_value()) {
    std::cerr << "Error: ActionTrigger 'type' field is required but not found"
              << '\n';
    return;
  }

  std::string type = type_opt.value();

  JsonObject params;
  if (trigger.contains("params")) {
    auto params_opt = get_value<ActionTrigger, JsonObject>(
        this->_registry.get(), trigger, entity, "params");

    if (!params_opt.has_value()) {
      std::cerr << "Warning: ActionTrigger 'params' could not be resolved"
                << '\n';
    } else {
      params = params_opt.value();
    }
  }

  std::vector<std::pair<std::string, JsonObject>> to_emit_map;

  if (obj.contains("to_emit")) {
    try {
      JsonArray emits_array = std::get<JsonArray>(obj.at("to_emit").value);
      for (auto const& emit_value : emits_array) {
        try {
          JsonObject emit_obj = std::get<JsonObject>(emit_value.value);
          for (auto const& [event_name, event_data_value] : emit_obj) {
            try {
              JsonObject event_data =
                  std::get<JsonObject>(event_data_value.value);
              to_emit_map.emplace_back(event_name, event_data);
            } catch (std::bad_variant_access const&) {
              std::cerr << "Error parsing action emit: event data is not a "
                           "JsonObject"
                        << '\n';
            }
          }
        } catch (std::bad_variant_access const&) {
          std::cerr << "Error parsing action emit: invalid format" << '\n';
        }
      }
    } catch (std::bad_variant_access const&) {
      std::cerr
          << "Error parsing action component: 'to_emit' is not a JsonArray"
          << '\n';
    }
  }

  init_component<ActionTrigger>(this->_registry.get(),
                                this->_event_manager.get(),
                                entity,
                                std::make_pair(type, params),
                                to_emit_map);
}

extern "C"
{
PLUGIN_EXPORT void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new Actions(r, em, e);
}
}
