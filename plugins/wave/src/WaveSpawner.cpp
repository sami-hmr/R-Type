#include <iostream>

#include "plugin/components/Wave.hpp"

#include "WaveManager.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/WaveTag.hpp"
#include "plugin/events/EntityManagementEvent.hpp"

void WaveManager::spawn_wave_entities(Registry::Entity wave_entity)
{
  auto& wave =
      this->_registry.get().get_components<Wave>()[wave_entity].value();

  Vector2D origin = wave.pattern.origin;
  if (this->_registry.get().has_component<Position>(wave_entity)) {
    origin = this->_registry.get().get_components<Position>()[wave_entity]->pos;
  }

  WavePattern adjusted_pattern = wave.pattern;
  adjusted_pattern.origin = origin;

  std::vector<Vector2D> positions =
      calculate_spawn_positions(adjusted_pattern, wave.count);

  LoadEntityTemplate::Additional inherited_components;
  for (auto const& comp_key : wave.components_inheritance) {
    auto comp_bytes =
        this->_registry.get().get_component_bytes(wave_entity, comp_key);
    if (comp_bytes.has_value()) {
      inherited_components.push_back({comp_key, comp_bytes.value()});
    }
  }

  Vector2D base_direction(1.0, 0.0);
  bool has_base_direction = false;

  for (auto const& [key, value] : inherited_components) {
    if (key == this->_registry.get().get_component_key<Direction>()) {
      Direction dir(value);
      base_direction = dir.direction;
      has_base_direction = true;
      break;
    }
  }

  for (int i = 0; i < wave.count; ++i) {
    Vector2D spawn_pos = origin;
    if (i < static_cast<int>(positions.size())) {
      spawn_pos = positions[i];
    }

    Vector2D formation_offset = spawn_pos - origin;

    LoadEntityTemplate::Additional entity_additionals {
        {this->_registry.get().get_component_key<Position>(),
         Position(spawn_pos).to_bytes()}};

    if (has_base_direction) {
      float angle_offset =
          calculate_direction_angle(wave.pattern, i, wave.count);
      Vector2D rotated_dir = rotate_direction(base_direction, angle_offset);
      entity_additionals.push_back(
          {this->_registry.get().get_component_key<Direction>(),
           Direction(rotated_dir).to_bytes()});
    }

    if (wave.tracked) {
      entity_additionals.push_back(
          {this->_registry.get().get_component_key<WaveTag>(),
           WaveTag(wave.id, formation_offset).to_bytes()});
    }

    for (auto const& [key, value] : inherited_components) {
      if (key != this->_registry.get().get_component_key<Direction>()
          && key != this->_registry.get().get_component_key<Position>())
      {
        entity_additionals.push_back({key, value});
      }
    }

    this->_event_manager.get().emit<LoadEntityTemplate>(wave.entity_template,
                                                        entity_additionals);
  }

  if (!wave.tracked) {
    this->_event_manager.get().emit<DeleteEntity>(wave_entity);
  }
}

void WaveManager::wave_spawn_system(Registry& r)
{
  for (auto&& [entity, wave, scene] : ZipperIndex<Wave, Scene>(r)) {
    if (r.is_entity_dying(entity)) {
      continue;
    }
    if (wave.spawned) {
      continue;
    }

    wave.spawned = true;
    this->_event_manager.get().emit<ComponentBuilder>(
        entity, r.get_component_key<Wave>(), wave.to_bytes());

    spawn_wave_entities(entity);
  }
}
