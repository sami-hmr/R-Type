#include <format>
#include <vector>

#include "BaseTypes.hpp"
#include "InventoryPlugin.hpp"
#include "Json/JsonParser.hpp"
#include "ParserTypes.hpp"
#include "ParserUtils.hpp"
#include "ecs/Entity.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/components/Text.hpp"
#include "plugin/events/EntityManagementEvent.hpp"

static std::string get_team_name(Ecs::Entity e, std::size_t slot)
{
  return std::format("__inventory_ath__:{}:{}", e, slot);
}

void InventoryPlugin::generate_ath_scene(GenerateInventoryScene const& e)
{
  if (this->_active_ath.contains(e.entity)) {
    this->delete_ath_scene(e.entity);
  }
  this->_active_ath.emplace(e.entity);

  auto const& inventory =
      *this->_registry.get().get_components<Inventory>()[e.entity];
  std::vector<std::string> items(inventory.max_items, "-----");

  for (std::size_t i = 0; i < inventory.inventory.size(); i++) {
    if (i >= inventory.max_items) {
      break;
    }
    items[i] = std::format(
        "{}: {}", inventory.inventory[i].item_name, inventory.inventory[i].nb);
  }

  for (std::size_t i = 0; i < items.size(); i++) {
    double height = 0.1;  // NOLINT
    double width = 1.0 / items.size();  // NOLINT
    double pos = -1.0 + 2.0 * (i + 0.5) / items.size();
    auto additionals = LoadEntityTemplate::Additional(
        {{this->_registry.get().get_component_key<Team>(),
          Team(get_team_name(e.entity, i)).to_bytes()}});
    if (this->_registry.get().has_component<Scene>(e.entity)) {
      additionals.emplace_back(this->_registry.get().get_component_key<Scene>(),
                               Scene(this->_registry.get()
                                         .get_components<Scene>()[e.entity]
                                         ->scene_name)
                                   .to_bytes());
    }
    this->_loader.get().load_entity_template(
        "card",
        additionals,
        JsonObject({{"width", width},
                    {"height", height},
                    {"x", pos},
                    {"y", 0.9},  // NOLINT
                    {"z", 20},
                    {"text", items[i]},
                    {"text_size",
                     JsonValue(JsonObject({{"height", height - 0.05},
                                           {"width", width - 0.05}}))}}));
  }
}

static Parser<std::size_t> parse_inventory_team()
{
  return apply([](std::size_t s) { return s; },
               parseUntil(':') >> parseChar(':') >> parseLongUnsignedInt()
                   >> parseChar(':') >> parseLongUnsignedInt());
}

void InventoryPlugin::update_ath_scenes()
{
  if (this->_active_ath.empty()) {
    return;
  }
  for (auto&& [e, team, text] : ZipperIndex<Team, Text>(this->_registry.get()))
  {
    if (!team.name.starts_with("__inventory_ath__")) {
      continue;
    }
    for (auto const& e : this->_active_ath) {
      if (!this->_registry.get().has_component<Inventory>(e)) {
        continue;
      }

      if (!team.name.starts_with(std::format("__inventory_ath__:{}", e))) {
        continue;
      }
      auto r = parse_inventory_team()(team.name);
      if (r.index() == ERR) {
        continue;
      }
      auto const slot = std::get<SUCCESS>(r).value;
      auto const& inventory =
          *this->_registry.get().get_components<Inventory>()[e];
      if (slot >= inventory.inventory.size()) {
        text.text = "-----";
        continue;
      }
      text.text = std::format("{}: {}",
                              inventory.inventory[slot].item_name,
                              inventory.inventory[slot].nb);
    }
  }
}

void InventoryPlugin::delete_ath_scene(GenerateInventoryScene const& event)
{
  if (!this->_active_ath.contains(event.entity)) {
    return;
  }
  for (auto const&& [e, team] : ZipperIndex<Team>(this->_registry.get())) {
    if (team.name.starts_with(
            std::format("__inventory_ath__:{}", event.entity)))
    {
      this->_registry.get().kill_entity(e);
    }
  }
  this->_active_ath.erase(event.entity);
}
