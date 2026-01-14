#pragma once

#include <string>
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/components/Item.hpp"
#include "plugin/events/EventMacros.hpp"
#include "plugin/events/InventoryEvents.hpp"

#define DFLT_MAX 64

class InventoryPlugin : public APlugin
{
public:
  InventoryPlugin(Registry& r, EventManager& em, EntityLoader& l);
  ~InventoryPlugin() override = default;
  EntityLoader& entity_loader;

private:
  std::vector<std::pair<Item, std::size_t>> _inventory;
  std::size_t _max_items;

  void use_item(std::uint8_t slot, std::size_t nb);
  void add_item(const Item& item, std::size_t nb);

  void init_item_vector(Registry::Entity const& entity, JsonArray& inventory);

  void init_inventory(Registry::Entity const& entity, JsonObject const& obj);

  template<typename T>
  bool usage_emit(const ItemEvent<T>& event, std::string area);
};
