#include <optional>
#include <utility>

#include "Artefacts.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Item.hpp"
#include "plugin/events/CollisionEvent.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/InventoryEvents.hpp"
#include "plugin/events/LogMacros.hpp"
#include "plugin/events/LoggerEvent.hpp"

Artefacts::Artefacts(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("collision",
              r,
              em,
              l,
              {"collision", "inventory"},
              {COMP_INIT(PickableTool, PickableTool, init_pickable_artefacts)})
    , entity_loader(l)
{
  REGISTER_COMPONENT(PickableTool)

  SUBSCRIBE_EVENT(CollisionEvent, { on_pickable_interaction(event); })
}

void Artefacts::init_pickable_artefacts(Registry::Entity const& entity,
                                        JsonObject const& obj)
{
  auto const& on_consumption = get_value<PickableTool, JsonObject>(
      this->_registry.get(), obj, entity, "on_consumption");
  auto const& on_throw = get_value<PickableTool, JsonObject>(
      this->_registry.get(), obj, entity, "on_throw");
  auto const& name = get_value<PickableTool, std::string>(
      this->_registry.get(), obj, entity, "name");
  auto const& consumable = get_value<PickableTool, bool>(
      this->_registry.get(), obj, entity, "consumable");
  auto const& throwable = get_value<PickableTool, bool>(
      this->_registry.get(), obj, entity, "throwable");

  if (!name || !consumable || !!throwable) {
    std::cerr << "Error loading Temporal artefact: missing a field\n";
    return;
  }
  if ((*consumable && !on_consumption) || (*throwable && !on_throw)) {
    LOGGER("Artefacts",
           LogLevel::ERR,
           "Missing Event field for either consume or throw")
    return;
  }

  this->_registry.get().emplace_component<PickableTool>(
      entity, on_consumption, on_throw, *name, *consumable, *throwable);
}

void Artefacts::on_pickable_interaction(CollisionEvent const& event)
{
  for (auto&& [artefact, tool] :
       ZipperIndex<PickableTool>(this->_registry.get()))
  {
    if ((event.a == artefact || event.b == artefact)
        && _registry.get().has_component<Inventory>(
            event.a == artefact ? event.b : event.a))
    {
      this->_event_manager.get().emit<PickUp>(
          artefact_to_item(tool), 1, event.a == artefact ? event.b : event.a);
      this->_event_manager.get().emit<DeleteEntity>(artefact);
    }
  }
}

Item Artefacts::artefact_to_item(PickableTool artefact)
{
  std::pair<std::string, JsonObject> obj;
  JsonObject config;

  obj.first = artefact.name;
  if (!artefact.consumable && !artefact.throwable) {
    return {obj, artefact.consumable, artefact.throwable};
  }
  if (artefact.consumable) {
    config.insert_or_assign("consume", JsonValue(*artefact.on_consumption));
  }
  if (artefact.throwable) {
    config.insert_or_assign("throw", JsonValue(*artefact.on_throw));
  }
  obj.second = config;
  return {obj, artefact.consumable, artefact.throwable};
}
