#include <optional>
#include <utility>

#include "Artefacts.hpp"

#include "Json/JsonParser.hpp"
#include "artefacts/WeaponEffect.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/components/Item.hpp"
#include "plugin/events/CollisionEvent.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/InventoryEvents.hpp"

Artefacts::Artefacts(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("collision",
              r,
              em,
              l,
              {"collision", "inventory"},
              {COMP_INIT(HealArtefact,
                         HealArtefact,
                         init_temporal_artefacts<HealArtefact>),
               COMP_INIT(PoisonArtefact,
                         PoisonArtefact,
                         init_temporal_artefacts<PoisonArtefact>),
               COMP_INIT(SpeedUpArtefact,
                         SpeedArtefact,
                         init_temporal_artefacts<SpeedArtefact>)})
    , entity_loader(l)
{
  REGISTER_COMPONENT(HealArtefact)
  REGISTER_COMPONENT(PoisonArtefact)
  REGISTER_COMPONENT(SpeedArtefact)

  SUBSCRIBE_EVENT(CollisionEvent, {
    on_temporal_effect<HealArtefact>(event, "HealEvent", "amount");
    on_temporal_effect<PoisonArtefact>(event, "DamageEvent", "amount");
    on_temporal_effect<SpeedArtefact>(
        event, "SpeedModifierEvent", "multiplier");
  })
}

template<typename T>
void Artefacts::init_temporal_artefacts(Registry::Entity const& entity,
                                        JsonObject const& obj)
{
  auto const& activate_on = get_value<T, ActivationEffect>(
      this->_registry.get(), obj, entity, "activate_on");
  auto const& effective_time = get_value<T, double>(
      this->_registry.get(), obj, entity, "effective_time");
  auto const& name =
      get_value<T, std::string>(this->_registry.get(), obj, entity, "name");
  auto const& consumable =
      get_value<T, bool>(this->_registry.get(), obj, entity, "consumable");
  auto const& throwable =
      get_value<T, bool>(this->_registry.get(), obj, entity, "throwable");
  auto const& points =
      get_value<T, double>(this->_registry.get(), obj, entity, "points");

  if (!activate_on || !effective_time || !name || !consumable || throwable
      || !points)
  {
    std::cerr << "Error loading Temporal artefact: missing a field\n";
    return;
  }

  this->_registry.get().emplace_component<T>(entity,
                                             std::nullopt,
                                             *activate_on,
                                             *effective_time,
                                             *name,
                                             *consumable,
                                             *throwable,
                                             *points);
}

template<typename T>
void Artefacts::on_temporal_effect(CollisionEvent const& event,
                                   const std::string& evt_name,
                                   const std::string& param_field)
{
  for (auto&& [artefact, effect] :
       ZipperIndex<TemporalEffect<T>>(this->_registry.get()))
  {
    if ((event.a == artefact || event.b == artefact)
        && _registry.get().has_component<Inventory>(
            event.a == artefact ? event.b : event.a))
    {
      JsonObject params;
      params.insert_or_assign(
          "entity",
          JsonValue(static_cast<int>(event.a == artefact ? event.b : event.a)));
      params.insert_or_assign(param_field, JsonValue(effect.points));
      params.insert_or_assign("source", JsonValue(static_cast<int>(artefact)));
      this->_event_manager.get().emit<PickUp>(
          artefact_to_item(effect, evt_name, params),
          1,
          event.a == artefact ? event.b : event.a);
      this->_event_manager.get().emit<DeleteEntity>(artefact);
    }
  }
}

template<typename T>
Item Artefacts::artefact_to_item(TemporalEffect<T> artefact,
                                 std::string evt_name,
                                 JsonObject evt_params)
{
  std::pair<std::string, JsonObject> obj;
  JsonObject consumption;
  JsonObject throwption;
  JsonObject config;
  JsonObject evt;

  obj.first = artefact.name;
  obj.second = config;
  if (artefact.activate_on == NOACTIVATION) {
    return Item(obj, artefact.consumable, artefact.throwable);
  }
  evt.insert_or_assign("name", JsonValue(std::move(evt_name)));
  evt.insert_or_assign("params", JsonValue(evt_params));
  if (artefact.activate_on != THROW) {
    consumption.insert_or_assign("event", JsonValue(evt));
    config.insert_or_assign("consume", JsonValue(consumption));
  }
  if (artefact.activate_on != CONSUMPTION) {
    throwption.insert_or_assign("event", JsonValue(evt));
    config.insert_or_assign("throw", JsonValue(throwption));
  }
  return Item(obj, artefact.consumable, artefact.throwable);
}

template<typename T>
TemporalEffect<T> Artefacts::item_to_temporal_artefact(Item const& item)
{
  ActivationEffect activate_on;
  std::string name;
  bool throwable;
  bool consumable;
  double points;



  return TemporalEffect<T>();
}
