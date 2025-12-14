#pragma once

#include <string>

#include "libs/Vector2D.hpp"

struct Spawner
{
  Spawner() = default;

  Spawner(Vector2D spawn_position,
             std::string entity_template,
             double spawn_interval,
             int max_spawns)
      : spawn_position(spawn_position)
      , entity_template(std::move(entity_template))
      , spawn_interval(spawn_interval)
      , spawn_delta(0.0)
      , max_spawns(max_spawns)
      , current_spawns(0)
      , active(true)
  {
  }

  Vector2D spawn_position;
  std::string entity_template;
  double spawn_interval;
  double spawn_delta;
  int max_spawns;
  int current_spawns;
  bool active;
};
