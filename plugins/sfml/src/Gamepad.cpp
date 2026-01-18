#include <optional>

#include <SFML/Window/Joystick.hpp>

#include "Json/JsonParser.hpp"
#include "SFMLRenderer.hpp"
#include "ecs/EmitEvent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/components/Controllable.hpp"
#include "plugin/events/CollisionEvent.hpp"
#include "plugin/events/IoEvents.hpp"
#include "plugin/events/WeaponEvent.hpp"

void SFMLRenderer::gamepad_system(Registry& r)
{
  std::array<JoystickState, sf::Joystick::Count> current_states;

  for (unsigned int i = 0; i < sf::Joystick::Count; ++i) {
    if (!sf::Joystick::isConnected(i)) {
      continue;
    }
    std::array<double, sf::Joystick::AxisCount> axes;
    std::array<bool, sf::Joystick::ButtonCount> buttons;
    for (unsigned int j = 0; j < sf::Joystick::AxisCount; j++) {
      if (sf::Joystick::hasAxis(i, static_cast<sf::Joystick::Axis>(j))) {
        double deadzone = 15;
        double position = sf::Joystick::getAxisPosition(
            i, static_cast<sf::Joystick::Axis>(j));
        double norm = position;
        if (std::abs(norm) < deadzone) {
          norm = 0.0;
        }
        axes[j] = norm;
      }
    }
    for (unsigned int j = 0; j < sf::Joystick::ButtonCount; j++) {
      buttons[j] = sf::Joystick::isButtonPressed(i, j);
    }
    current_states[i] = JoystickState(axes, buttons);
  }
  for (auto&& [e, control] : ZipperIndex<Controllable>(r)) {
    for (auto& [key, trigger] : control.gamepad_event_map) {
      if (!GAMEPAD_AXIS_MAP.contains(static_cast<GamePad::Keys>(key))
          && !GAMEPAD_BUTTON_MAP.contains(static_cast<GamePad::Keys>(key)))
      {
        continue;
      }
      _joysticks[0] = current_states[0];
      if (GAMEPAD_AXIS_MAP.contains(static_cast<GamePad::Keys>(key))) {
        std::pair<sf::Joystick::Axis, sf::Joystick::Axis> axis =
            GAMEPAD_AXIS_MAP.at(static_cast<GamePad::Keys>(key));
        double value_x =
            current_states[0].axes[static_cast<size_t>(axis.first)];
        double value_y =
            current_states[0].axes[static_cast<size_t>(axis.second)];
        if (trigger.first.first == "SetDirectionEvent") {
          emit_event<SetDirectionEvent>(
              this->_event_manager.get(),
              trigger.first.first,
              SetDirectionEvent(e, Vector2D(value_x, value_y).normalize()));
        }
        continue;
      }
      if (GAMEPAD_BUTTON_MAP.contains(static_cast<GamePad::Keys>(key))) {
        unsigned int button =
            GAMEPAD_BUTTON_MAP.at(static_cast<GamePad::Keys>(key));
        bool is_pressed = current_states[0].buttons[button];
        if (is_pressed) {
          try {
            emit_event(this->_event_manager.get(),
                       r,
                       trigger.first.first,
                       trigger.second,
                       e);
          } catch (std::bad_optional_access) {
            continue;
          }
        }
        continue;
      }
    }
  }
}
