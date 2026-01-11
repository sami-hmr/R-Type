#include <iostream>

#include "WaveManager.hpp"

#include "ecs/InitComponent.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "patterns/ArcPattern.hpp"
#include "patterns/CirclePattern.hpp"
#include "patterns/FormationVPattern.hpp"
#include "patterns/GridPattern.hpp"
#include "patterns/LinePattern.hpp"
#include "patterns/PointPattern.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Formation.hpp"
#include "plugin/components/Wave.hpp"
#include "plugin/components/WaveTag.hpp"
#include "plugin/events/DeathEvent.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/WaveEvent.hpp"

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
  REGISTER_COMPONENT(WaveTag)

  _patterns[WavePatternType::POINT] = std::make_unique<PointPattern>();
  _patterns[WavePatternType::LINE] = std::make_unique<LinePattern>();
  _patterns[WavePatternType::CIRCLE] = std::make_unique<CirclePattern>();
  _patterns[WavePatternType::ARC] = std::make_unique<ArcPattern>();
  _patterns[WavePatternType::GRID] = std::make_unique<GridPattern>();
  _patterns[WavePatternType::FORMATION_V] =
      std::make_unique<FormationVPattern>();

  this->_registry.get().add_system([this](Registry& r)
                                   { wave_formation_system(r); });
  this->_registry.get().add_system([this](Registry& r)
                                   { wave_spawn_system(r); });
  this->_registry.get().add_system([this](Registry& r)
                                   { wave_death_system(r); });

  SUBSCRIBE_EVENT(WaveSpawnEvent, {
    for (auto &&entity: event.wave_templates) {
      this->_event_manager.get().emit<LoadEntityTemplate>(
          entity, LoadEntityTemplate::Additional {});
    }
  })
}

std::optional<Registry::Entity> WaveManager::find_wave_by_id(std::size_t id)
{
  std::optional<Registry::Entity> wave_opt;

  for (auto&& [wave_entity, wave] : ZipperIndex<Wave>(this->_registry.get())) {
    if (wave.id == id) {
      wave_opt.emplace(wave_entity);
    }
  }
  return wave_opt;
}

std::size_t WaveManager::generate_wave_id()
{
  std::size_t new_id = _next_wave_id++;

  while (find_wave_by_id(new_id).has_value()) {
    new_id = _next_wave_id++;
  }

  return new_id;
}

void WaveManager::init_wave(Registry::Entity const& entity,
                            JsonObject const& obj)
{
  auto entity_template = get_value<Wave, std::string>(
      this->_registry.get(), obj, entity, "entity_template");
  auto count =
      get_value<Wave, int>(this->_registry.get(), obj, entity, "count");
  auto tracked =
      get_value<Wave, bool>(this->_registry.get(), obj, entity, "tracked");

  if (!entity_template) {
    std::cerr << "Error loading Wave component: missing entity_template\n";
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

    if (pattern_obj.contains("origin")) {
      auto origin_opt = get_value_copy<Vector2D>(
          this->_registry.get(), pattern_obj, "origin");
      if (origin_opt) {
        pattern.origin = origin_opt.value();
      }
    }

    if (pattern_obj.contains("params")) {
      auto params_obj = std::get<JsonObject>(pattern_obj.at("params").value);
      pattern.params = params_obj;
    }
  } else {
    std::cerr << "Error loading Wave component: missing pattern\n";
    return;
  }

  OnEndEvent on_end;
  if (obj.contains("on_end")) {
    auto on_end_obj = std::get<JsonObject>(obj.at("on_end").value);

    if (on_end_obj.contains("event_name")) {
      auto name_str = get_value_copy<std::string>(
          this->_registry.get(), on_end_obj, "event_name");
      if (!name_str) {
        std::cerr
            << "Error loading Wave component: missing on_end event_name\n";
        return;
      }
      on_end.event_name = name_str.value();
    }

    if (on_end_obj.contains("params")) {
      auto params_obj = std::get<JsonObject>(on_end_obj.at("params").value);
      on_end.params = params_obj;
    }
  } else {
    std::cerr << "Error loading Wave component: missing on_end\n";
    return;
  }

  std::size_t wave_id = generate_wave_id();
  bool is_tracked = tracked.value_or(true);

  std::vector<std::string> inheritance;
  if (obj.contains("components_inheritance")) {
    auto const& inherit_array =
        std::get<JsonArray>(obj.at("components_inheritance").value);
    for (auto const& item : inherit_array) {
      inheritance.push_back(std::get<std::string>(item.value));
    }
  }

  this->_registry.get().emplace_component<Wave>(entity,
                                                wave_id,
                                                entity_template.value(),
                                                count.value_or(1),
                                                pattern,
                                                on_end,
                                                is_tracked,
                                                false,
                                                inheritance);
}

void WaveManager::init_formation(Registry::Entity const& entity,
                                 JsonObject const& obj)
{
  auto strength = get_value<Formation, double>(
      this->_registry.get(), obj, entity, "strength");

  if (!strength) {
    std::cerr << "Error loading Formation component: missing strength\n";
    return;
  }

  this->_registry.get().emplace_component<Formation>(entity, strength.value());
}

extern "C"
{
void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new WaveManager(r, em, e);
}
}
