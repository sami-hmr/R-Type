#include <iterator>

#include "Controller.hpp"

#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"
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

Key Controller::char_to_key(char c)
{
  auto it = mapping.find(c);
  if (it != mapping.end()) {
    return it->second;
  }
  return Key::Unknown;
}

Controller::Controller(Registery &r, EntityLoader& l)
    : APlugin(r, l, {"moving"}, {COMP_INIT(Controllable, init_controller)})
{
  this->_registery.get().register_component<Controllable>();

  this->_registery.get().on<KeyPressedEvent>(
      [this](const KeyPressedEvent &c)
      {
        for (auto const &[key, active] : c.key_pressed) {
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
                                 JsonVariant const& config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);

    std::string up_str = std::get<std::string>(obj.at("UP").value);
    std::string down_str = std::get<std::string>(obj.at("DOWN").value);
    std::string left_str = std::get<std::string>(obj.at("LEFT").value);
    std::string right_str = std::get<std::string>(obj.at("RIGHT").value);

    if (up_str.empty() || down_str.empty() || left_str.empty()
        || right_str.empty())
    {
      LOGGER("Controller",
             LogLevel::ERROR,
             "Controllable keys cannot be empty strings")
      return;
    }

    this->_registery.get().emplace_component<Controllable>(
        entity, up_str[0], down_str[0], left_str[0], right_str[0]);

  } catch (std::bad_variant_access const&) {
    LOGGER("Controller",
           LogLevel::ERROR,
           "Error loading Controllable component: unexpected value type")
  } catch (std::out_of_range const&) {
    LOGGER("Controller",
           LogLevel::ERROR,
           "Error loading Controllable component: missing keys (expected UP, "
           "DOWN, LEFT, RIGHT)")
  }
}

void Controller::handle_key_change(Key key, bool is_pressed)
{
  this->_key_states[key] = is_pressed;

  auto &velocities = this->_registery.get().get_components<Velocity>();
  auto const &controllers =
      this->_registery.get().get_components<Controllable>();

  for (auto &&[controller, velocity] : Zipper(controllers, velocities)) {
    Key up_key = this->char_to_key(controller.up);
    Key down_key = this->char_to_key(controller.down);
    Key left_key = this->char_to_key(controller.left);
    Key right_key = this->char_to_key(controller.right);

    velocity.direction.y = this->compute_axis(up_key, down_key);
    velocity.direction.x = this->compute_axis(left_key, right_key);
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
void* entry_point(Registery &r, EntityLoader &e)
{
  return new Controller(r, e);
}
}
