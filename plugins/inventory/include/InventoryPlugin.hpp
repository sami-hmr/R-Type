#pragma once

#include <unordered_map>
#include <unordered_set>

#include "Json/JsonParser.hpp"
#include "ecs/Entity.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/components/Inventory.hpp"
#include "plugin/components/Item.hpp"
#include "plugin/events/InventoryEvents.hpp"

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
  void init_pickable(Ecs::Entity const& entity, JsonObject const& obj);

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
  static const std::size_t pick_delta = 1000000000;

  void generate_ath_scene(GenerateInventoryScene const&);
  void delete_ath_scene(GenerateInventoryScene const&);
  void update_ath_scenes();
  std::unordered_set<Ecs::Entity> _active_ath;
};
