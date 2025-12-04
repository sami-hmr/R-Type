#include "Controller.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Controllable.hpp"
#include "plugin/events/Events.hpp"

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

std::optional<Key> Controller::char_to_key(char c)
{
  auto it = mapping.find(c);
  if (it != mapping.end()) {
    return it->second;
  }
  return std::nullopt;
}

Controller::Controller(Registery& r, EntityLoader& l)
    : APlugin(r, l, {"moving"}, {COMP_INIT(Controllable, Controllable, init_controller)})
{
  this->_registery.get().register_component<Controllable>();

  this->_registery.get().on<KeyPressedEvent>(
      [this](const KeyPressedEvent& c)
      {
        for (auto const& [key, active] : c.key_pressed) {
          if (active) {
            this->handle_key_change(key, true);
          }
        }
      });

  this->_registery.get().on<KeyReleasedEvent>(
      [this](const KeyReleasedEvent& c)
      {
        for (auto const& [key, active] : c.key_released) {
          if (active) {
            this->handle_key_change(key, false);
          }
        }
      });
}

void Controller::init_controller(Registery::Entity const entity,
                                 JsonObject const& obj)
{
  auto const& up_str = get_value<std::string>(this->_registery, obj, "UP");
  auto const& down_str = get_value<std::string>(this->_registery, obj, "DOWN");
  auto const& left_str = get_value<std::string>(this->_registery, obj, "LEFT");
  auto const& right_str =
      get_value<std::string>(this->_registery, obj, "RIGHT");

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

  this->_registery.get().emplace_component<Controllable>(
      entity, up_str.value()[0], down_str.value()[0], left_str.value()[0], right_str.value()[0]);
}

void Controller::handle_key_change(Key key, bool is_pressed)
{
  auto& velocities = this->_registery.get().get_components<Velocity>();
  auto const& controllers =
      this->_registery.get().get_components<Controllable>();

  for (auto&& [controller, velocity] : Zipper(controllers, velocities)) {
    auto up_key = char_to_key(controller.up);
    auto down_key = char_to_key(controller.down);
    auto left_key = char_to_key(controller.left);
    auto right_key = char_to_key(controller.right);

    double dir = is_pressed ? 1.0 : 0.0;

    if (up_key.has_value() && key == up_key.value()) {
      velocity.dir_y = is_pressed ? -dir : 0.0;
    }
    if (down_key.has_value() && key == down_key.value()) {
      velocity.dir_y = is_pressed ? dir : 0.0;
    }
    if (left_key.has_value() && key == left_key.value()) {
      velocity.dir_x = is_pressed ? -dir : 0.0;
    }
    if (right_key.has_value() && key == right_key.value()) {
      velocity.dir_x = is_pressed ? dir : 0.0;
    }
  }
};

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new Controller(r, e);
}
}
