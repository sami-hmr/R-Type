#pragma once

#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/components/Item.hpp"

#define MAX_ITEMS 64

class Inventory : public APlugin
{
public:
  Inventory(Registry& r, EventManager& em, EntityLoader& l);
  ~Inventory() override = default;
  EntityLoader& entity_loader;

private:
  std::vector<std::pair<Item, std::size_t>> _inventory;
  std::size_t _max_items = MAX_ITEMS;

  void use_item(std::uint8_t slot, std::size_t nb, Registry::Entity e);
  void add_item(Item& item, std::size_t nb, Registry::Entity e);
};
