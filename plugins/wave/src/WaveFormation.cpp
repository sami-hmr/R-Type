#include "plugin/components/Wave.hpp"

#include "WaveManager.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/components/Formation.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/WaveTag.hpp"
#include "plugin/events/EntityManagementEvent.hpp"

void WaveManager::wave_formation_system(Registry& r)
{
  auto dt = r.clock().delta_seconds();

  for (auto&& [wave_entity, wave, formation] : ZipperIndex<Wave, Formation>(r))
  {
    if (r.is_entity_dying(wave_entity) || !formation.active) {
      continue;
    }

    std::vector<Registry::Entity> wave_entities;
    for (auto&& [entity, tag] : ZipperIndex<WaveTag>(r)) {
      if (tag.wave_id == wave.id && !r.is_entity_dying(entity)
          && r.has_component<Position>(entity))
      {
        wave_entities.push_back(entity);
      }
    }

    if (wave_entities.empty()) {
      continue;
    }

    Vector2D center(0, 0);
    for (auto entity_id : wave_entities) {
      auto& pos = r.get_components<Position>()[entity_id];
      center += pos->pos;
    }
    center = center / static_cast<double>(wave_entities.size());

    for (auto entity_id : wave_entities) {
      auto& pos_opt = r.get_components<Position>()[entity_id];
      auto& wave_tag = r.get_components<WaveTag>()[entity_id];

      Vector2D desired_pos = center + wave_tag->formation_offset;

      Vector2D to_target = desired_pos - pos_opt->pos;
      pos_opt->pos +=
          to_target * formation.strength * dt;

      this->_event_manager.get().emit<ComponentBuilder>(
          entity_id, r.get_component_key<Position>(), pos_opt->to_bytes());
    }
  }
}
