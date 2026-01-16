#include <cmath>

#include "WaveManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Wave.hpp"

Vector2D WaveManager::rotate_direction(Vector2D const& dir, float angle_rad)
{
  double cos_a = std::cos(angle_rad);
  double sin_a = std::sin(angle_rad);
  return Vector2D(dir.x * cos_a - dir.y * sin_a, dir.x * sin_a + dir.y * cos_a);
}

float WaveManager::calculate_direction_angle(WavePattern const& pattern,
                                             int index,
                                             int total_count)
{
  return _patterns[pattern.type]->calculate_direction_angle(
      this->_registry.get(), index, total_count, pattern.params);
}

std::vector<Vector2D> WaveManager::calculate_spawn_positions(
    const WavePattern& pattern, int count)
{
  std::vector<Vector2D> positions;
  positions.reserve(count);

  for (int i = 0; i < count; ++i) {
    positions.push_back(calculate_pattern_position(pattern, i, count));
  }

  return positions;
}

Vector2D WaveManager::calculate_pattern_position(const WavePattern& pattern,
                                                 int index,
                                                 int total_count)
{
  return _patterns[pattern.type]->calculate_position(
      this->_registry.get(), pattern.origin, index, total_count, pattern.params);
}
