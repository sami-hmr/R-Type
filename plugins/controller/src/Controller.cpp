#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <iterator>
#include <vector>

#include "Controller.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/EmitEvent.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Controllable.hpp"
#include "plugin/events/CollisionEvent.hpp"
#include "plugin/events/IoEvents.hpp"
#include "plugin/events/LoggerEvent.hpp"

Controller::Controller(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("Controller",
              r,
              em,
              l,
              {"logger"},
              {COMP_INIT(Controllable, Controllable, init_controller)})
{
  REGISTER_COMPONENT(Controllable)

  SUBSCRIBE_EVENT(KeyPressedEvent, {
    for (auto const& [key, active] : event.key_pressed) {
      if (active) {
        this->handle_key_change(key, true);
      }
    }
  });

  SUBSCRIBE_EVENT(KeyReleasedEvent, {
    for (auto const& [key, active] : event.key_released) {
      if (active) {
        this->handle_key_change(key, false);
      }
    }
  })
}

bool Controller::handling_press_release_binding(Registry::Entity const& entity,
                                                Controllable& result,
                                                JsonObject& event,
                                                const std::string& key_string,
                                                KeyEventType event_type)
{
  auto event_id =
      get_value_copy<std::string>(this->_registry.get(), event, "name");
  auto params =
      get_value_copy<JsonObject>(this->_registry.get(), event, "params");
  if (!event_id) {
    LOGGER("Controller",
           LogLevel::WARNING,
           "Missing name field in event, skipping");
    return false;
  }
  if (!params) {
    LOGGER("Controller",
           LogLevel::WARNING,
           std::format("Missing params field in event \"{}\", skipping",
                       *event_id));
    return false;
  }
  params->insert_or_assign("entity", JsonValue(static_cast<int>(entity)));
  result.event_map.insert_or_assign(
      (static_cast<std::uint32_t>(KEY_MAPPING.at_first(key_string)) << 8)
          + static_cast<int>(event_type),
      Controllable::Trigger {*event_id, *params});
  return true;
}

void Controller::init_event_map(Registry::Entity const& entity,
                                JsonArray& events,
                                Controllable& result)
{
  for (auto& it : events) {
    auto& event = std::get<JsonObject>(it.value);
    auto description = get_value_copy<std::string>(
        this->_registry.get(), event, "description"); // insert description into the bindings
    auto key_string =
        get_value_copy<std::string>(this->_registry.get(), event, "key");
    auto press =
        get_value_copy<JsonObject>(this->_registry.get(), event, "pressed");
    auto release =
        get_value_copy<JsonObject>(this->_registry.get(), event, "released");
    if (!description) {
      LOGGER("Controller",
             LogLevel::WARNING,
             std::format("Missing description field in event, skipping"));
      continue;
    }
    if (!key_string) {
      LOGGER("Controller",
             LogLevel::WARNING,
             std::format("Missing key field in event, skipping"));
      continue;
    }
    if (!release && !press) {
      LOGGER("Controller",
             LogLevel::WARNING,
             std::format("No action linked to command \"{}\".", *key_string))
      continue;
    }
    if (press) {
      if (!handling_press_release_binding(
        entity, result, *press, *key_string, KEY_PRESSED)) {
        continue;
      }
    }
    if (release) {
      if (!handling_press_release_binding(
        entity, result, *release, *key_string, KEY_RELEASED)) {
        continue;
      }
    }
  }
}

void Controller::init_controller(Registry::Entity const& entity,
                                 JsonObject const& obj)
{
  Controllable result(
      (std::unordered_map<std::uint16_t, Controllable::Trigger>()));
  auto bindings = std::get<JsonArray>(obj.at("bindings").value);

  this->init_event_map(entity, bindings, result);

  this->_registry.get().add_component<Controllable>(entity, std::move(result));
}

void Controller::handle_key_change(Key key, bool is_pressed)
{
  this->_key_states[key] = is_pressed;

  std::uint16_t key_map =
      (static_cast<std::uint32_t>(key) << 8) + static_cast<int>(is_pressed);

  std::vector<std::function<void()>> to_emit;
  for (auto&& [c] : Zipper<Controllable>(this->_registry.get())) {
    if (!c.event_map.contains(key_map)) {
      continue;
    }
    auto const& event = c.event_map.at(key_map);

    to_emit.emplace_back(
        [this, &event]()
        {
          emit_event(this->_event_manager.get(),
                     this->_registry.get(),
                     event.first,
                     event.second);
        });
  }
  for (auto const &it : to_emit) {
      it();
  }
};

bool Controller::is_key_active(Key target) const
{
  auto it = this->_key_states.find(target);
  return it != this->_key_states.end() && it->second;
}

double Controller::compute_axis(Key negative, Key positive) const
{
  bool negative_active =
      negative != Key::Unknown && this->is_key_active(negative);
  bool positive_active =
      positive != Key::Unknown && this->is_key_active(positive);

  if (negative_active == positive_active) {
    return 0.0;
  }
  return negative_active ? -1.0 : 1.0;
}

extern "C"
{
void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new Controller(r, em, e);
}
}
