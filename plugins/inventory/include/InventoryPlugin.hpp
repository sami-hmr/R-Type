#pragma once

#include "Json/JsonParser.hpp"
#include "ecs/Entity.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/components/Inventory.hpp"
#include "plugin/components/Item.hpp"

class InventoryPlugin : public APlugin
{
public:
  InventoryPlugin(Registry& r, EventManager& em, EntityLoader& l);
  ~InventoryPlugin() override = default;

private:
  static void delete_item(std::uint8_t slot,
                          std::size_t nb,
                          Inventory& inventory);

  std::vector<std::pair<Item, std::size_t>> init_item_vector(
      JsonArray& inventory);

  void init_inventory(Ecs::Entity const& entity, JsonObject const& obj);

  void drop_item(Inventory& inventory,
                 std::size_t consumer,
                 std::uint8_t slot_item,
                 std::size_t nb_to_use);
  void remove_item(Inventory& inventory,
                   std::uint8_t slot_item,
                   std::size_t nb_to_use);
  void pick_item(Inventory& inventory, Pickable const& to_pick, Ecs::Entity);
  void use_item(Inventory& inventory,
                std::size_t consumer,
                std::uint8_t slot_item,
                std::size_t nb_to_use);
};
