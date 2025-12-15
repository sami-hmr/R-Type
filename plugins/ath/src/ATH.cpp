#include <optional>

#include "ATH.hpp"

#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Bar.hpp"
#include "plugin/components/Clickable.hpp"
#include "plugin/events/IoEvents.hpp"

ATH::ATH(Registry& r, EntityLoader& l, std::optional<JsonObject> const& config)
    : APlugin("ath",
              r,
              l,
              {"ui"},
              {COMP_INIT(Bar, Bar, init_bar),
               COMP_INIT(Clickable, Clickable, init_clickable)},
              config)
{
  REGISTER_COMPONENT(Bar)
  REGISTER_COMPONENT(Clickable)
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
