#pragma once

#include <cmath>
#include <functional>
#include <memory>
#include <unordered_map>

#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "patterns/IPatternStrategy.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Formation.hpp"
#include "plugin/components/Wave.hpp"

class WaveManager : public APlugin
{
public:
  WaveManager(Registry& r, EventManager& em, EntityLoader& l);

private:
  void init_wave(Registry::Entity const& entity, JsonObject const& obj);
  void init_formation(Registry::Entity const& entity, JsonObject const& obj);

  void spawn_wave_entities(Registry::Entity wave_entity);

  void wave_formation_system(Registry& r);
  void wave_spawn_system(Registry& r);
  void wave_death_system(Registry& r);

  std::optional<Registry::Entity> find_wave_by_id(std::size_t id);
  std::size_t generate_wave_id();
  
  std::vector<Vector2D> calculate_spawn_positions(const WavePattern& pattern,
                                                  int count);
  Vector2D calculate_pattern_position(const WavePattern& pattern,
                                      int index,
                                      int total_count);
  
  Vector2D rotate_direction(Vector2D const& dir, float angle_rad);
  float calculate_direction_angle(WavePattern const& pattern, 
                                   int index, 
                                   int total_count);

  std::size_t _next_wave_id = 1;
  std::unordered_map<WavePatternType, std::unique_ptr<IPatternStrategy>> _patterns;
};
