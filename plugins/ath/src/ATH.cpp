#include "ATH.hpp"

#include <optional>

#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Bar.hpp"
#include "plugin/events/IoEvents.hpp"

ATH::ATH(Registry& r, EntityLoader& l, std::optional<JsonObject> const &config)
    : APlugin("ath", r, l, {"ui"}, {COMP_INIT(Bar, Bar, init_bar)})
{
  _registry.get().register_component<Bar>("ath:Bar");
  SUBSCRIBE_EVENT(MousePressedEvent, {
    std::cout << "Mouse button pressed at ("
              << event.position.x << ", " << event.position.y << ")"
              << " button: " << static_cast<int>(event.button) << std::endl;
  })
}

extern "C"
{
void* entry_point(Registry& r,
                  EntityLoader& e,
                  std::optional<JsonObject> const& config)
{
  return new ATH(r, e, config);
}
}
