#include <format>
#include <iterator>

#include "Controller.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Controllable.hpp"
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
    : APlugin(r,
              l,
              {"logger", "moving"},
              {COMP_INIT(Controllable, Controllable, init_controller)})
{
  this->_registry.get().register_component<Controllable>(
      "controller:Controllable");

  this->_registry.get().on<KeyPressedEvent>(
      "KeyPressedEvent",
      [this](const KeyPressedEvent& c)
      {
        for (auto const& [key, active] : c.key_pressed) {
          if (active) {
            this->handle_key_change(key, true);
          }
        }
      });

  this->_registry.get().on<KeyReleasedEvent>(
      "KeyReleasedEvent",
      [this](const KeyReleasedEvent& c)
      {
        for (auto const& [key, active] : c.key_released) {
          if (active) {
            this->handle_key_change(key, false);
          }
        }
      });
}

void Controller::init_controller(Registry::Entity const entity,
                                 JsonObject const& obj)
{
  auto const& up_str =
      get_value<Controllable, std::string>(this->_registry, obj, entity, "UP");
  auto const& down_str = get_value<Controllable, std::string>(
      this->_registry, obj, entity, "DOWN");
  auto const& left_str = get_value<Controllable, std::string>(
      this->_registry, obj, entity, "LEFT");
  auto const& right_str = get_value<Controllable, std::string>(
      this->_registry, obj, entity, "RIGHT");

  if (!up_str || !down_str || !left_str || !right_str) {
    std::cerr << "Error loading controller component: unexpected value type "
                 "(expected UP, " "DOWN, LEFT, RIGHT)\n";
    return;
  }

  if (up_str->empty() || down_str->empty() || left_str->empty()
      || right_str->empty())
  {
    LOGGER("Controller",
           LogLevel::ERROR,
           "Controllable keys cannot be empty strings")
    return;
  }

  this->_registry.get().emplace_component<Controllable>(entity,
                                                        up_str.value()[0],
                                                        down_str.value()[0],
                                                        left_str.value()[0],
                                                        right_str.value()[0]);
}

void Controller::handle_key_change(Key key, bool is_pressed)
{
  this->_key_states[key] = is_pressed;

  auto const& controllers =
      this->_registry.get().get_components<Controllable>();

  for (auto&& [index, controller] : ZipperIndex(controllers)) {
    Key up_key = this->char_to_key(controller.up);
    Key down_key = this->char_to_key(controller.down);
    Key left_key = this->char_to_key(controller.left);
    Key right_key = this->char_to_key(controller.right);

    this->_registry.get().emit<UpdateVelocity>(
        index,
        this->compute_axis(left_key, right_key),
        this->compute_axis(up_key, down_key));
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
