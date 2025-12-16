#include <cstdint>
#include <format>
#include <iterator>

#include "Controller.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Controllable.hpp"
#include "plugin/components/Velocity.hpp"
#include "plugin/events/CollisionEvent.hpp"
#include "plugin/events/IoEvents.hpp"
#include "plugin/events/LoggerEvent.hpp"

static const std::map<char, Key> mapping = {
    {'z', Key::Z},
    {'Z', Key::Z},
    {'q', Key::Q},
    {'Q', Key::Q},
    {'s', Key::S},
    {'S', Key::S},
    {'d', Key::D},
    {'D', Key::D},
    {'r', Key::R},
    {'R', Key::R},
    {' ', Key::SPACE},
    {'\n', Key::ENTER},
    {'\033', Key::ECHAP},
    {'\b', Key::DELETE},
};

Key Controller::char_to_key(char c)
{
  auto it = mapping.find(c);
  if (it != mapping.end()) {
    return it->second;
  }
  return Key::Unknown;
}

Controller::Controller(Registry& r, EntityLoader& l)
    : APlugin("controller",
              r,
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

void Controller::init_controller(Registry::Entity const entity,
                                 JsonObject const& obj)
{
    Controllable result((std::unordered_map<std::uint16_t, Controllable::Trigger>()));
    auto pressed = std::get<JsonArray>(obj.at("pressed").value);
    auto released = std::get<JsonArray>(obj.at("released").value);

    for (auto &it : pressed) {

    }
}

void Controller::handle_key_change(Key key, bool is_pressed)
{
  this->_key_states[key] = is_pressed;
  std::uint16_t key_map = (key << 8) + is_pressed;

  for (auto&& [c] : Zipper<Controllable>(this->_registry.get())) {
    if (!c.event_map.contains(key_map)) {
      continue;
    }
    auto const &event = c.event_map.at(key_map);
    this->_registry.get().emit(event.first, event.second);
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
void* entry_point(Registry& r, EntityLoader& e)
{
  return new Controller(r, e);
}
}
