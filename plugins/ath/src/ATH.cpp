#include <optional>

#include "ATH.hpp"

#include "ecs/EmitEvent.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "libs/Rect.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Bar.hpp"
#include "plugin/components/Button.hpp"
#include "plugin/components/Clickable.hpp"
#include "plugin/components/Collidable.hpp"
#include "plugin/events/IoEvents.hpp"

static void on_click(Registry& r,
                     EventManager& em,
                     const MousePressedEvent& event)
{
  for (const auto& [draw, clickable, pos, collision] :
       Zipper<Drawable, Clickable, Position, Collidable>(r))
  {
    if (!draw.enabled) {
      continue;
    }
    Rect entity_rect = {.x = pos.pos.x,
                        .y = pos.pos.y,
                        .width = collision.width,
                        .height = collision.height};
    if (entity_rect.contains(event.position.x, event.position.y)) {
      for (const auto& [name, obj] : clickable.to_emit) {
        emit_event(em, r, name, obj);
      }
    }
  }
}

ATH::ATH(Registry& r,
         EventManager& em,
         EntityLoader& l,
         std::optional<JsonObject> const& config)
    : APlugin("ath",
              r,
              em,
              l,
              {"ui"},
              {COMP_INIT(Bar, Bar, init_bar),
               COMP_INIT(Clickable, Clickable, init_clickable),
               COMP_INIT(Button, Button, init_button)},
              config)
{
  REGISTER_COMPONENT(Bar)
  REGISTER_COMPONENT(Clickable)
  REGISTER_COMPONENT(Button)

  SUBSCRIBE_EVENT(MousePressedEvent, {
    on_click(this->_registry.get(), this->_event_manager.get(), event);
  });
}

extern "C"
{
void* entry_point(Registry& r,
                  EventManager& em,
                  EntityLoader& e,
                  std::optional<JsonObject> const& config)
{
  return new ATH(r, em, e, config);
}
}
