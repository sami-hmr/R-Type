#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "InventoryPlugin.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/EmitEvent.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Byte.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Item.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Speed.hpp"
#include "plugin/events/CreateEntity.hpp"
#include "plugin/events/DeathEvent.hpp"
#include "plugin/events/InventoryEvents.hpp"
#include "plugin/events/LogMacros.hpp"

InventoryPlugin::InventoryPlugin(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("inventory",
              r,
              em,
              l,
              {"moving", "artefacts"},
              {COMP_INIT(Inventory, Inventory, init_inventory)})
    , entity_loader(l)
{
  REGISTER_COMPONENT(Item);
  REGISTER_COMPONENT(Inventory);

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
          return usage_emit(
              "consume", inventory.slots[event.slot_item].first.object.second);
        }
      }
    }
  })
  SUBSCRIBE_EVENT(DeathEvent, {
    auto inventory = _registry.get().get_components<Inventory>()[event.entity];
    if (!inventory) {
      return false;
    }
    std::size_t i = 0;
    for (auto item : (*inventory).slots) {
      this->_event_manager.get().emit<Drop>(i, true, 1, event.entity);
      i += 1;
    }
  })
  SUBSCRIBE_EVENT(Drop, {
    for (auto&& [entity, inventory] :
         ZipperIndex<Inventory>(this->_registry.get()))
    {
      if (entity == event.consumer) {
        if (event.slot_item >= inventory.slots.size()) {
          return false;
        }
        use_item(event.slot_item, event.nb_to_use, inventory);
        if (inventory.slots[event.slot_item].first.throwable) {
          create_artefact(inventory.slots[event.slot_item].first, entity, {});
          return false;
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
          // std::vector<std::pair<std::string, ByteArray>> others;
          if (this->_registry.get().has_component<Speed>(entity)) {
            // others.push_back(
            //     std::make_pair(this->_registry.get().get_component_key<Speed>(),
            //                    this->_registry.get()
            //                        .get_components<Position>()[entity]
            //                        .value()
            //                        .to_bytes()));
          }
          create_artefact(inventory.slots[event.slot_item].first, entity, {});
          // create_artefact(inventory.slots[event.slot_item].first, entity, others);
          return usage_emit(
              "throw", inventory.slots[event.slot_item].first.object.second);
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
      if (entity == event.possessor) {
        add_item(event.item, event.quantity, inventory);
      }
    }
  })
}

PickableTool InventoryPlugin::item_to_artefact(Item const& item)
{
  std::string name = item.object.first;
  auto on_consumption = get_value_copy<JsonObject>(
      this->_registry.get(), item.object.second, "consume");
  auto on_throw = get_value_copy<JsonObject>(
      this->_registry.get(), item.object.second, "throw");

  return {on_consumption, on_throw, name, item.consumable, item.throwable};
}

void InventoryPlugin::create_artefact(
    Item const& item,
    Registry::Entity entity,
    std::vector<std::pair<std::string, ByteArray>> const& other_components)
{
  PickableTool artefact = item_to_artefact(item);
  ByteArray artefact_bytes = artefact.to_bytes();

  this->_event_manager.get().emit<ComponentBuilder>(
      entity, this->_registry.get().get_component_key<Item>(), artefact_bytes);
  CreateEntity::Additional additional = {
      {this->_registry.get().get_component_key<PickableTool>(),
       artefact_bytes}};

  if (this->_registry.get().has_component<Position>(entity)) {
    additional.push_back({this->_registry.get().get_component_key<Position>(),
                          this->_registry.get()
                              .get_components<Position>()[entity]
                              .value()
                              .to_bytes()});
  }
  if (this->_registry.get().has_component<Scene>(entity)) {
    additional.push_back({this->_registry.get().get_component_key<Scene>(),
                          this->_registry.get()
                              .get_components<Scene>()[entity]
                              .value()
                              .to_bytes()});
  }
  for (auto& cmp : other_components) {
    additional.push_back(cmp);
  }
  LOGGER("Inventory Plugin", LogLevel::WARNING, "Creating an entity...")
  this->_event_manager.get().emit<CreateEntity>(additional);
}

std::vector<std::pair<Item, std::size_t>> InventoryPlugin::init_item_vector(
    JsonArray& inventory)
{
  std::vector<std::pair<Item, std::size_t>> slots;

  for (auto& it : inventory) {
    auto& item = std::get<JsonObject>(it.value);
    auto name =
        get_value_copy<std::string>(this->_registry.get(), item, "name");
    auto quantity =
        get_value_copy<int>(this->_registry.get(), item, "quantity");
    auto consumable =
        get_value_copy<bool>(this->_registry.get(), item, "consumable");
    auto throwable =
        get_value_copy<bool>(this->_registry.get(), item, "throwable");
    auto config =
        get_value_copy<JsonObject>(this->_registry.get(), item, "config");
    if (!name) {
      LOGGER("InventoryPlugin",
             LogLevel::WARNING,
             std::format("Missing field name in item, skipping"));
      continue;
    }
    if (!quantity) {
      LOGGER("InventoryPlugin",
             LogLevel::WARNING,
             std::format("Missing field quantity in item, skipping"));
      continue;
    }
    if (!consumable) {
      LOGGER("InventoryPlugin",
             LogLevel::WARNING,
             std::format("Missing field consumable in item, skipping"));
      continue;
    }
    if (!throwable) {
      LOGGER("InventoryPlugin",
             LogLevel::WARNING,
             std::format("Missing field throwable in item, skipping"));
      continue;
    }
    slots.emplace_back(
        Item(std::make_pair(*name, *config), *consumable, *throwable),
        static_cast<std::size_t>(*quantity));
  }
  return slots;
}

void InventoryPlugin::init_inventory(Registry::Entity const& entity,
                                     JsonObject const& obj)
{
  std::size_t max_items = std::get<int>(obj.at("max_items").value);
  auto inventory = std::get<JsonArray>(obj.at("items").value);
  std::vector<std::pair<Item, std::size_t>> items =
      this->init_item_vector(inventory);
  std::string all_content;
  for (auto const& item : items) {
    all_content.append(item.first.object.first + " x"
                       + std::to_string(item.second) + ";");
  }
  init_component<Inventory>(this->_registry.get(),
                            this->_event_manager.get(),
                            entity,
                            items,
                            all_content,
                            entity,
                            max_items);
}

bool InventoryPlugin::usage_emit(std::string area, const JsonObject& obj)
{
  auto use_item = get_value_copy<JsonObject>(this->_registry.get(), obj, area);
  auto entity = get_value_copy<int>(this->_registry.get(), obj, "entity");
  if (!use_item || !entity) {
    LOGGER("InventoryPlugin",
           LogLevel::ERR,
           std::format("Missing {} field in item. No event played", area));
    return false;
  }
  auto name =
      get_value_copy<std::string>(this->_registry.get(), *use_item, "name");
  auto params =
      get_value_copy<JsonObject>(this->_registry.get(), *use_item, "params");
  if (!name || !params) {
    LOGGER(
        "InventoryPlugin",
        LogLevel::ERR,
        std::format(
            "Invalid event field in item's {} configuration. No even't played",
            area));
    return false;
  }
  emit_event(this->_event_manager.get(), this->_registry.get(), *name, *params);
  return false;
}

void InventoryPlugin::add_item(const Item& item,
                               std::size_t nb,
                               Inventory& inventory)
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

void InventoryPlugin::use_item(std::uint8_t slot,
                               std::size_t nb,
                               Inventory& inventory)
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
