#include <optional>

#include "ATH.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/EmitEvent.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Rect.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Bar.hpp"
#include "plugin/components/Button.hpp"
#include "plugin/components/Clickable.hpp"
#include "plugin/components/Collidable.hpp"
#include "plugin/components/Input.hpp"
#include "plugin/events/IoEvents.hpp"

static void on_click(Registry& r,
                     EventManager& em,
                     const MousePressedEvent& event)
{
  std::vector<std::function<void(void)>> to_emit;
  for (const auto& [e, draw, clickable, pos, collision] :
       ZipperIndex<Drawable, Clickable, Position, Collidable>(r))
  {
    if (!draw.enabled || !r.is_in_main_scene(e)) {
      continue;
    }
    Rect entity_rect = {.x = pos.pos.x,
                        .y = pos.pos.y,
                        .width = collision.size.x * 2,
                        .height = collision.size.y * 2};
    if (entity_rect.contains(event.position.x, event.position.y)) {
      for (auto&& [name, obj] : clickable.to_emit) {
        std::cout << "Clickable: entity " << e << " clicked, emitting '" << name
                  << "'\n";
        obj.insert_or_assign("entity", JsonVariant(static_cast<int>(e)));
        // Capture name and obj by value to avoid dangling references
        to_emit.emplace_back([&em, &r, name, obj]()
                             { emit_event(em, r, name, obj); });
        std::cout << "Clickable: emitted event '" << name << "'\n";
      }
    }
  }
  for (auto const& f : to_emit) {
    f();
  }
}

void on_input_focus(Registry& r, const InputFocusEvent& event)
{
  for (const auto& [e, input] : ZipperIndex<Input>(r)) {
    if (e == event.entity) {
      input.enabled = !input.enabled;
    } else {
      input.enabled = false;
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
  SUBSCRIBE_EVENT(InputFocusEvent,
                  { on_input_focus(this->_registry.get(), event); });
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
