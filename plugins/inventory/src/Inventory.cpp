#include "Inventory.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/EmitEvent.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Item.hpp"
#include "plugin/events/InventoryEvents.hpp"
#include "plugin/events/LoggerEvent.hpp"

template<typename T>
bool Inventory::usage_emit(const ItemEvent<T>& event, std::string area)
{
  use_item(event.slot_item, event.nb_to_use);
  auto use_item = get_value_copy<JsonObject>(
      this->_registry.get(),
      _inventory[event.slot_item].first.object.second,
      area);
  auto entity =
      get_value_copy<int>(this->_registry.get(),
                          _inventory[event.slot_item].first.object.second,
                          "entity");
  if (!use_item || !entity) {
    LOGGER(
        "Inventory",
        LogLevel::ERROR,
        std::format("Missing {} field in item. No animation nor event played",
                    area));
    return false;
  }
  auto evt_use =
      get_value_copy<JsonObject>(this->_registry.get(), *use_item, "event");
  if (evt_use) {
    auto name =
        get_value_copy<std::string>(this->_registry.get(), *evt_use, "name");
    auto params =
        get_value_copy<JsonObject>(this->_registry.get(), *evt_use, "params");
    if (!name || !params) {
      LOGGER("Inventory",
             LogLevel::ERROR,
             std::format("Invalid event field in item's {} configuration. No "
                         "animation nor even't played",
                         area));
      return false;
    }
    params->insert_or_assign("entity", JsonValue(*entity));
    emit_event(
        this->_event_manager.get(), this->_registry.get(), *name, *params);
  }
  return false;
}

Inventory::Inventory(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin(
          "inventory", r, em, l, {}, {COMP_INIT(Item, Item, init_inventory)})
    , entity_loader(l)
    , _max_items(DFLT_MAX)
{
  SUBSCRIBE_EVENT(Consume, {
    if (event.slot_item >= _inventory.size()) {
      return false;
    }
    if (_inventory[event.slot_item].first.consumable) {
      return usage_emit<ConsumeItem>(event, "consume");
    }
  })
  SUBSCRIBE_EVENT(Throw, {
    if (event.slot_item >= _inventory.size()) {
      return false;
    }
    if (_inventory[event.slot_item].first.throwable) {
      return usage_emit<ThrowItem>(event, "throw");
    }
  })
  SUBSCRIBE_EVENT(Remove, {
    if (event.slot_item >= _inventory.size()) {
      return false;
    }
    use_item(event.slot_item, event.nb_to_use);
  })
  SUBSCRIBE_EVENT(PickUp, { add_item(event.item, event.nb_to_use); })
}

void Inventory::init_item_vector(Registry::Entity const& entity,
                                 JsonArray& inventory)
{
  for (auto& it : inventory) {
    auto& item = std::get<JsonObject>(it.value);
    auto name =
        get_value_copy<std::string>(this->_registry.get(), item, "name");
    auto consumable =
        get_value_copy<bool>(this->_registry.get(), item, "consumable");
    auto throwable =
        get_value_copy<bool>(this->_registry.get(), item, "throwable");
    auto quantity =
        get_value_copy<std::size_t>(this->_registry.get(), item, "quantity");
    auto config =
        get_value_copy<JsonObject>(this->_registry.get(), item, "config");
    if (!name || !quantity || !consumable || !throwable) {
      LOGGER("Inventory",
             LogLevel::WARNING,
             std::format("Missing a field in item, skipping"));
      continue;
    }
    config->insert_or_assign("entity", JsonValue(static_cast<int>(entity)));
    this->add_item(
        Item(std::make_pair(*name, *config), *consumable, *throwable),
        *quantity);
  }
}

void Inventory::init_inventory(Registry::Entity const& entity,
                               JsonObject const& obj)
{
  _max_items = std::get<int>(obj.at("max_items").value);
  auto inventory = std::get<JsonArray>(obj.at("items").value);
  this->init_item_vector(entity, inventory);
}

void Inventory::add_item(const Item& item, std::size_t nb)
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
  if (_inventory.size() >= _max_items) {
    return;
  }
  _inventory.emplace_back(item, nb);
}

void Inventory::use_item(std::uint8_t slot, std::size_t nb)
{
  if (nb < _inventory[slot].second) {
    _inventory[slot].second -= nb;
  } else {
    _inventory.erase(_inventory.begin() + slot);
  }
}

extern "C"
{
void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new Inventory(r, em, e);
}
}
