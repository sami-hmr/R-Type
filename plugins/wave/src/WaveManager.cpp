#include <cmath>
#include <iostream>

#include "WaveManager.hpp"

#include "NetworkShared.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Formation.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Wave.hpp"
#include "plugin/events/DeathEvent.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/WaveEvents.hpp"

WaveManager::WaveManager(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("wave",
              r,
              em,
              l,
              {"moving"},
              {COMP_INIT(Wave, Wave, init_wave),
               COMP_INIT(Formation, Formation, init_formation)})
{
  REGISTER_COMPONENT(Wave)
  REGISTER_COMPONENT(Formation)

  this->_registry.get().add_system([this](Registry& r)
                                   { wave_spawn_system(r); });
  this->_registry.get().add_system([this](Registry& r)
                                   { wave_formation_system(r); });
}

void WaveManager::init_wave(Registry::Entity const& entity,
                            JsonObject const& obj)
{
  auto entity_template = get_value<Wave, std::string>(
      this->_registry.get(), obj, entity, "entity_template");
  auto count =
      get_value<Wave, int>(this->_registry.get(), obj, entity, "count");

  if (!entity_template || !count) {
    std::cerr
        << "Error loading Wave component: missing entity_template or " "count"
                                                                       "\n";
    return;
  }

  WavePattern pattern;
  if (obj.contains("pattern")) {
    auto pattern_obj = std::get<JsonObject>(obj.at("pattern").value);

    if (pattern_obj.contains("type")) {
      auto type_str = get_value_copy<std::string>(
          this->_registry.get(), pattern_obj, "type");
      if (!type_str) {
        std::cerr << "Error loading Wave component: missing pattern type\n";
        return;
      }
      pattern.type = parse_wave_pattern_type(type_str.value());
    }
    pattern.params = pattern_obj;
  } else {
    std::cerr << "Error loading Wave component: missing pattern\n";
    return;
  }

  init_component<Wave>(this->_registry.get(),
                       this->_event_manager.get(),
                       entity,
                       entity_template.value(),
                       count.value(),
                       pattern);
}

void WaveManager::init_formation(Registry::Entity const& entity,
                                 JsonObject const& obj)
{
  auto strength = get_value<Formation, float>(
      this->_registry.get(), obj, entity, "strength");

  if (!strength) {
    std::cerr << "Error loading Formation component: missing strength\n";
    return;
  }

  init_component<Formation>(this->_registry.get(),
                            this->_event_manager.get(),
                            entity,
                            strength.value());
}

void WaveManager::wave_spawn_system(Registry& r)
{
  for (auto&& [i, wave] : ZipperIndex<Wave>(r)) {
    if (r.is_entity_dying(i) || !wave.active || !wave.spawned_entities.empty())
    {
      continue;
    }
  }
}

void WaveManager::wave_formation_system(Registry& r)
{
  for (auto&& [wave_entity, wave, formation] : ZipperIndex<Wave, Formation>(r))
  {
    if (r.is_entity_dying(wave_entity) || !formation.active
        || wave.spawned_entities.empty())
    {
      continue;
    }
  }
}
