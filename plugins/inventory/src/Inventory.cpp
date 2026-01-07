#include "Inventory.hpp"

#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Item.hpp"
#include "plugin/events/InventoryEvents.hpp"

Inventory::Inventory(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("inventory", r, em, l, {}, {})
    , entity_loader(l)
{
  SUBSCRIBE_EVENT(Consume, {
    if (event.slot_item >= _inventory.size()) {
      return false;
    }
    if (_inventory[event.slot_item].first.consumable) {
      use_item(event.slot_item, event.nb_to_use, event.consumer);
    }
  })
  SUBSCRIBE_EVENT(Throw, {
    if (event.slot_item >= _inventory.size()) {
      return false;
    }
    if (_inventory[event.slot_item].first.throwable) {
      use_item(event.slot_item, event.nb_to_use, event.consumer);
    }
  })
  SUBSCRIBE_EVENT(Remove, {
    if (event.slot_item >= _inventory.size()) {
      return false;
    }
    use_item(event.slot_item, event.nb_to_use, event.consumer);
  })
  // SUBSCRIBE_EVENT(PickUp, {
  //   if (event.item >= _inventory.size()) {
  //     return false;
  //   }
  //   add_item(event.item, event.nb_to_use, event.consumer);
  // })
}

void Inventory::add_item(Item& item, std::size_t nb, Registry::Entity e)
{
  bool added = false;

  for (auto& it : _inventory) {
    if (item.object.first == it.first.object.first) {
      it.second += nb;
      added = true;
      break;
    }
  }
  if (added) {
    return;
  }
  if (_inventory.size() >= MAX_ITEMS) {
    return;
  }
  _inventory.emplace_back(item, nb);
}

void Inventory::use_item(std::uint8_t slot, std::size_t nb, Registry::Entity e)
{
  // std::size_t used = nb;
  if (nb < _inventory[slot].second) {
    _inventory[slot].second -= nb;
  } else {
    _inventory.erase(_inventory.begin() + slot);
  }
  // call le callback de l'objet consommé

  // _event_manager.get().emit<EventType>(
  //     item.object,
  //     used,
  //     0);  // change last arg to be the consumer (can't find out how to
  // retrieve it for the moment)

  // faire subscribe tout élément contenant l'attribut item, étant
  // throwable/consumable et au player ayant consommé
}
