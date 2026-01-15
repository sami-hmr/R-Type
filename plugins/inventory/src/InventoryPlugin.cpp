#include <utility>
#include <vector>

#include "ecs/Registry.hpp"
#include "ecs/EmitEvent.hpp"
#include "plugin/APlugin.hpp"
#include "Json/JsonParser.hpp"
#include "InventoryPlugin.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/InitComponent.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Item.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/events/InventoryEvents.hpp"

InventoryPlugin::InventoryPlugin(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("inventory",
              r,
              em,
              l,
              {},
              {COMP_INIT(Inventory, Inventory, init_inventory)})
    , entity_loader(l)
{
  REGISTER_COMPONENT(Item)
  REGISTER_COMPONENT(Inventory)

  SUBSCRIBE_EVENT(Consume, {
    for (auto&& [entity, inventory] :
         ZipperIndex<Inventory>(this->_registry.get()))
    {
      if (entity == event.consumer) {
        if (event.slot_item >= inventory.slots.size()) {
          return false;
        }
        use_item(event.slot_item, event.nb_to_use, inventory);
        if (inventory.slots[event.slot_item].first.consumable) {
          return usage_emit<ConsumeItem>(
              "consume",
              inventory.slots[event.slot_item].first.object.second);
        }
      }
    }
  })
  SUBSCRIBE_EVENT(Throw, {
    for (auto&& [entity, inventory] :
         ZipperIndex<Inventory>(this->_registry.get()))
    {
      if (entity == event.consumer) {
        if (event.slot_item >= inventory.slots.size()) {
          return false;
        }
        use_item(event.slot_item, event.nb_to_use, inventory);
        if (inventory.slots[event.slot_item].first.throwable) {
          return usage_emit<ThrowItem>(
              "throw",
              inventory.slots[event.slot_item].first.object.second);
        }
      }
    }
  })
  SUBSCRIBE_EVENT(Remove, {
    for (auto&& [entity, inventory] :
         ZipperIndex<Inventory>(this->_registry.get()))
    {
      if (entity == event.consumer) {
        if (event.slot_item >= inventory.slots.size()) {
          return false;
        }
        use_item(event.slot_item, event.nb_to_use, inventory);
      }
    }
  })
  SUBSCRIBE_EVENT(PickUp, {
    for (auto&& [entity, inventory] :
         ZipperIndex<Inventory>(this->_registry.get()))
    {
      if (entity == event.consumer) {
        add_item(event.item, event.nb_to_use, inventory);
      }
    }
  })
}

std::vector<std::pair<Item, std::size_t>> InventoryPlugin::init_item_vector(Registry::Entity const& entity,
                                       JsonArray& inventory)
{
  std::vector<std::pair<Item, std::size_t>> slots;

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
      LOGGER("InventoryPlugin",
             LogLevel::WARNING,
             std::format("Missing a field in item, skipping"));
      continue;
    }
    config->insert_or_assign("entity", JsonValue(static_cast<int>(entity)));
    slots.emplace_back(
        Item(std::make_pair(*name, *config), *consumable, *throwable),
        *quantity);
  }
  return slots;
}

void InventoryPlugin::init_inventory(Registry::Entity const& entity,
                                     JsonObject const& obj)
{
  std::size_t max_items = std::get<int>(obj.at("max_items").value);
  auto inventory = std::get<JsonArray>(obj.at("items").value);
  std::vector<std::pair<Item, std::size_t>> items = this->init_item_vector(entity, inventory);
// create all_content by using all names + nb of items
  init_component<Inventory>(this->_registry.get(),
                            this->_event_manager.get(),
                            entity,
                            items,
                            "",// add all content instead
                            entity,
                            max_items);
}

template<typename T>
bool InventoryPlugin::usage_emit(std::string area,
                                 const JsonObject& obj)
{
  auto use_item = get_value_copy<JsonObject>(this->_registry.get(), obj, area);
  auto entity = get_value_copy<int>(this->_registry.get(), obj, "entity");
  if (!use_item || !entity) {
    LOGGER(
        "InventoryPlugin",
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
      LOGGER(
          "InventoryPlugin",
          LogLevel::ERROR,
          std::format(
              "Invalid event field in item's {} configuration. No " "animat" "i" "on" " n" "or" " " "even't " "played",
              area));
      return false;
    }
    params->insert_or_assign("entity", JsonValue(*entity));
    emit_event(
        this->_event_manager.get(), this->_registry.get(), *name, *params);
  }
  return false;
}

void InventoryPlugin::add_item(const Item& item, std::size_t nb, Inventory &inventory)
{
  bool added = false;

  for (auto& it : inventory.slots) {
    if (item.object.first == it.first.object.first) {
      it.second += nb;
      added = true;
      break;
    }
  }
  if (added) {
    return;
  }
  if (inventory.slots.size() >= inventory.max_items) {
    return;
  }
  inventory.slots.emplace_back(item, nb);
}

void InventoryPlugin::use_item(std::uint8_t slot, std::size_t nb, Inventory &inventory)
{
  if (nb < inventory.slots[slot].second) {
    inventory.slots[slot].second -= nb;
  } else {
    inventory.slots.erase(inventory.slots.begin() + slot);
  }
}

extern "C"
{
void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new InventoryPlugin(r, em, e);
}
}
