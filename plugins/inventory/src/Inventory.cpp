#include "Inventory.hpp"

#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Item.hpp"
#include "plugin/events/InventoryEvents.hpp"

Inventory::Inventory(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("inventory", r, em, l, {}, {})
    , entity_loader(l)
{
}

void Inventory::add_item(Item& item, std::size_t nb)
{
  bool added = false;

  for (auto& it : _inventory) {
    if (item.object == it.first.object) {
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
  _inventory.emplace(item, nb);
}

template<typename EventType>
void Inventory::use_item(const Item& item, std::size_t nb, bool usable)
{
  std::size_t thrown = nb;

  for (auto& it : _inventory) {
    if (item.object == it.first.object && usable) {
      if (nb < it.second) {
        it.second -= nb;
      } else {
        thrown = it.second;
        _inventory.erase(it.first);
      }
      _event_manager.get().emit<EventType>(
          item.object,
          thrown,
          0);  // change last arg to be the consumer (can't find out how to
               // retrieve it for the moment)

      // faire subscribe tout élément contenant l'attribut item, étant
      // throwable/consumable et au player ayant consommé
      return;
    }
  }
}

void Inventory::throw_item(const Item& item, std::size_t nb)
{
  use_item<Thrown>(item, nb, item.throwable);
}

void Inventory::consume_item(const Item& item, std::size_t nb)
{
  use_item<Consumed>(item, nb, item.consumable);
}

void Inventory::delete_item(const Item& item, std::size_t nb)
{
  use_item<Removed>(item, nb, true);

}
