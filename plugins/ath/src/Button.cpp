#include "plugin/components/Button.hpp"

#include "ATH.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/Hooks.hpp"

void ATH::init_button(Ecs::Entity const& e, JsonObject const& obj)
{
  bool pressed = false;
  bool hovered = false;
  bool toggle = false;

  if (obj.contains("pressed")) {
    pressed =
        get_value<Button, bool>(_registry.get(), obj, e, "pressed").value();
  }
  if (obj.contains("hovered")) {
    hovered =
        get_value<Button, bool>(_registry.get(), obj, e, "hovered").value();
  }
  if (obj.contains("toggle")) {
    toggle = get_value<Button, bool>(_registry.get(), obj, e, "toggle").value();
  }
  init_component<Button>(this->_registry.get(),
                         this->_event_manager.get(),
                         e,
                         pressed,
                         hovered,
                         toggle);
}
