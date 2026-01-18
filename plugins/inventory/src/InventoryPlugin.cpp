#include <algorithm>
#include <cstddef>
#include <exception>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "InventoryPlugin.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/EmitEvent.hpp"
#include "ecs/Entity.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Item.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/events/CollisionEvent.hpp"
#include "plugin/events/DeathEvent.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/InventoryEvents.hpp"
#include "plugin/events/LogMacros.hpp"

InventoryPlugin::InventoryPlugin(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("inventory",
              r,
              em,
              l,
              {"moving"},
              {COMP_INIT(Inventory, Inventory, init_inventory),
               COMP_INIT(Pickable, Pickable, init_pickable)})
{
  REGISTER_COMPONENT(Inventory);
  REGISTER_COMPONENT(Pickable);

  SUBSCRIBE_EVENT(DropItem, {
    if (!this->_registry.get().has_component<Inventory>(event.consumer)) {
      return false;
    }
    auto& inventory =
        *_registry.get().get_components<Inventory>()[event.consumer];

    this->drop_item(
        inventory, event.consumer, event.slot_item, event.nb_to_use);
  })
  SUBSCRIBE_EVENT(DeathEvent, {
    if (!this->_registry.get().has_component<Inventory>(event.entity)) {
      return false;
    }
    auto& inventory =
        *_registry.get().get_components<Inventory>()[event.entity];
    while (!inventory.inventory.empty()) {
      this->drop_item(inventory, event.entity, 0, inventory.inventory[0].nb);
    }
  })

  SUBSCRIBE_EVENT(RemoveItem, {
    if (!this->_registry.get().has_component<Inventory>(event.consumer)) {
      return false;
    }
    auto& inventory =
        *_registry.get().get_components<Inventory>()[event.consumer];
    this->remove_item(inventory, event.slot_item, event.nb_to_use);
  })

  SUBSCRIBE_EVENT(UseItem, {
    if (!this->_registry.get().has_component<Inventory>(event.consumer)) {
      return false;
    }
    auto& inventory =
        *_registry.get().get_components<Inventory>()[event.consumer];
    this->use_item(inventory, event.consumer, event.slot_item, event.nb_to_use);
  })

  SUBSCRIBE_EVENT(CollisionEvent, {
    this->_event_manager.get().emit<PickUp>(event.a, event.b);
  })

  SUBSCRIBE_EVENT(PickUp, {
    if (!this->_registry.get().has_component<Inventory>(event.picker)
        || !this->_registry.get().has_component<Pickable>(event.to_pick))
    {
      return false;
    }
    auto& inventory =
        *_registry.get().get_components<Inventory>()[event.picker];
    auto const& pickable =
        *_registry.get().get_components<Pickable>()[event.to_pick];

    if (pickable.delta + pick_delta
        < std::chrono::high_resolution_clock::now().time_since_epoch().count())
    {
      this->pick_item(inventory, pickable, event.to_pick);
    }
  })
}

void InventoryPlugin::init_inventory(Ecs::Entity const& entity,
                                     JsonObject const& obj)
{
  auto const& max_items = get_value<Inventory, int>(
      this->_registry.get(), obj, entity, "max_items");
  if (!max_items) {
    LOGGER_EVTLESS("inventory", "error", "missing max_items");
    return;
  }

  std::vector<Inventory::ItemSlot> inventory_slots;

  if (obj.contains("inventory")) {
    try {
      JsonArray inventory_array =
          get_value<Inventory, JsonArray>(
              this->_registry.get(), obj, entity, "inventory")
              .value();

      for (auto const& slot_value : inventory_array) {
        try {
          JsonObject slot_obj = std::get<JsonObject>(slot_value.value);

          std::string item_name =
              get_value<Inventory, std::string>(
                  this->_registry.get(), slot_obj, entity, "item_name")
                  .value();

          std::size_t nb = get_value<Inventory, int>(
                               this->_registry.get(), slot_obj, entity, "nb")
                               .value();

          JsonObject item_obj =
              get_value<Inventory, JsonObject>(
                  this->_registry.get(), slot_obj, entity, "item")
                  .value();

          Item item = Item(this->_registry.get(), item_obj, entity);

          std::string artefact_template =
              get_value<Inventory, std::string>(

                  this->_registry.get(), slot_obj, entity, "artefact_template")

                  .value();

          inventory_slots.emplace_back(item_name, nb, item, artefact_template);

        } catch (std::bad_variant_access const&) {
          std::cerr << "Error parsing inventory slot: invalid format" << '\n';
        }
      }

    } catch (std::bad_variant_access const&) {
      std::cerr << "Error parsing inventory: 'inventory' is not a JsonArray"
                << '\n';
      return;
    }
  }
  if (inventory_slots.size() > *max_items) {
    inventory_slots.erase(inventory_slots.begin() + *max_items,
                          inventory_slots.end());
  }
  this->_registry.get().emplace_component<Inventory>(
      entity, inventory_slots, *max_items);
}

void InventoryPlugin::init_pickable(Ecs::Entity const& entity,
                                    JsonObject const& obj)
{
  auto const& item_name = get_value<Pickable, std::string>(
      this->_registry.get(), obj, entity, "item_name");
  if (!item_name) {
    LOGGER_EVTLESS("pickable", "error", "missing item name");
    return;
  }
  auto const& artefact_template = get_value<Pickable, std::string>(
      this->_registry.get(), obj, entity, "artefact_template");
  if (!artefact_template) {
    LOGGER_EVTLESS("pickable", "error", "missing artefact template");
    return;
  }

  auto const& item = get_value<Pickable, JsonObject>(
      this->_registry.get(), obj, entity, "item");
  if (!item) {
    LOGGER_EVTLESS("pickable", "error", "missing item");
    return;
  }

  try {
    this->_registry.get().emplace_component<Pickable>(
        entity,
        *item_name,
        *artefact_template,
        Item(this->_registry.get(), *item, entity));
  } catch (std::exception const&) {
    LOGGER_EVTLESS("pickable", "error", "failed to init item");
  }
}

void InventoryPlugin::drop_item(Inventory& inventory,
                                std::size_t consumer,
                                std::uint8_t slot_item,
                                std::size_t nb_to_use)
{
  Position pos;
  if (this->_registry.get().has_component<Position>(consumer)) {
    pos = *this->_registry.get().get_components<Position>()[consumer];
  }
  if (inventory.inventory.size() <= slot_item) {
    LOGGER(
        "inventory",
        LogLevel::WARNING,
        std::format(
            "dropping out of range inventory item: inventory size: " "{}, " "tr" "yi" "n" "g " "to remove " "item {}",
            inventory.inventory.size(),
            slot_item))
    return;
  }
  nb_to_use = std::min(nb_to_use, inventory.inventory[slot_item].nb);
  for (std::size_t i = 0; i < nb_to_use; i++) {
    this->_event_manager.get().emit<LoadEntityTemplate>(
        inventory.inventory[slot_item].artefact_template,
        LoadEntityTemplate::Additional(
            {{this->_registry.get().get_component_key<Position>(),
              pos.to_bytes()}}));
  }
  this->remove_item(inventory, slot_item, nb_to_use);
}

void InventoryPlugin::remove_item(Inventory& inventory,
                                  std::uint8_t slot_item,
                                  std::size_t nb_to_use)
{
  if (inventory.inventory.size() <= slot_item) {
    LOGGER(
        "inventory",
        LogLevel::WARNING,
        std::format(
            "removing out of range inventory item: inventory size: " "{}, " "tr" "yi" "n" "g " "to remove " "item {}",
            inventory.inventory.size(),
            slot_item))
    return;
  }
  std::cout << "salem ? " << inventory.inventory[slot_item].nb << "   "
            << nb_to_use << "\n";
  if (inventory.inventory[slot_item].nb > nb_to_use) {
    inventory.inventory[slot_item].nb -= nb_to_use;
  } else {
    std::cout << "allo salem " << slot_item << "\n";
    inventory.inventory.erase(inventory.inventory.begin() + slot_item);
  }
}

void InventoryPlugin::pick_item(Inventory& inventory,
                                Pickable const& to_pick,
                                Ecs::Entity picked_entity)
{
  auto slot =
      std::find_if(inventory.inventory.begin(),
                   inventory.inventory.end(),
                   [name = to_pick.item_name](Inventory::ItemSlot const& it)
                   { return it.item_name == name; });
  if (slot == inventory.inventory.end()) {
    if (inventory.max_items <= inventory.inventory.size()) {
      return;
    }
    inventory.inventory.emplace_back(
        to_pick.item_name, 1, to_pick.item, to_pick.artefact_template);
  } else {
    slot->nb += 1;
  }
  this->_event_manager.get().emit<DeleteEntity>(picked_entity);
}

void InventoryPlugin::use_item(Inventory& inventory,
                               std::size_t consumer,
                               std::uint8_t slot_item,
                               std::size_t nb_to_use)
{
  if (inventory.inventory.size() <= slot_item) {
    LOGGER(
        "inventory",
        LogLevel::WARNING,
        std::format(
            "using out of range inventory item: inventory size: {}, " "tryin" "g " "to use " "item " "{} ",
            inventory.inventory.size(),
            slot_item))
    return;
  }
  nb_to_use = std::min(nb_to_use, inventory.inventory[slot_item].nb);
  std::cout << "nb to use" << nb_to_use << "\n";
  for (std::size_t i = 0; i < nb_to_use; i++) {
    for (auto const& it : inventory.inventory[slot_item].item.on_use) {
      for (auto const& [event, params] : it) {
        auto const* obj = std::get_if<JsonObject>(&params.value);
        if (obj != nullptr) {
          std::cout << event << "\n";
          this->_event_manager.get().emit(
              this->_registry.get(), event, *obj, consumer);
        } else {
          LOGGER(
              "inventory", LogLevel::WARNING, "invalid event confid: " + event);
        }
      }
    }
  }
  this->remove_item(inventory, slot_item, nb_to_use);
}

extern "C"
{
void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new InventoryPlugin(r, em, e);
}
}
