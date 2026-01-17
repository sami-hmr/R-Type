#pragma once

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/components/Inventory.hpp"
#include "plugin/components/Item.hpp"
#include "plugin/components/PickableTools.hpp"

#define DFLT_MAX 64

class InventoryPlugin : public APlugin
{
public:
  InventoryPlugin(Registry& r, EventManager& em, EntityLoader& l);
  ~InventoryPlugin() override = default;
  EntityLoader& entity_loader;

private:
  static void add_item(const Item& item, std::size_t nb, Inventory& inventory);
  static void use_item(std::uint8_t slot, std::size_t nb, Inventory& inventory);

  std::vector<std::pair<Item, std::size_t>> init_item_vector(JsonArray& inventory);

  void init_inventory(Registry::Entity const& entity, JsonObject const& obj);

  void create_artefact(Item const& item,
                       Registry::Entity entity);

  PickableTool item_to_artefact(Item const& item);

  bool usage_emit(std::string area, const JsonObject& obj);
};
