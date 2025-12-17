#pragma once

#include <string>

#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

struct Spawner
{
  Spawner() = default;

  Spawner(std::string entity_template, double spawn_interval, int max_spawns)
      : entity_template(std::move(entity_template))
      , spawn_interval(spawn_interval)
      , spawn_delta(0.0)
      , max_spawns(max_spawns)
      , current_spawns(0)
      , active(true)
  {
  }

  Spawner(std::string entity_template,
          double spawn_interval,
          double spawn_delta,
          int max_spawns,
          int current_spawns,
          bool active)
      : entity_template(std::move(entity_template))
      , spawn_interval(spawn_interval)
      , spawn_delta(spawn_delta)
      , max_spawns(max_spawns)
      , current_spawns(current_spawns)
      , active(active)
  {
  }

  HOOKABLE(Spawner,
           HOOK(entity_template),
           HOOK(spawn_interval),
           HOOK(spawn_delta),
           HOOK(max_spawns),
           HOOK(current_spawns),
           HOOK(active))

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(Spawner,
                           (
                               [](std::string entity_template,
                                  double spawn_interval,
                                  double spawn_delta,
                                  int max_spawns,
                                  int current_spawns,
                                  bool active)
                               {
                                 return Spawner(entity_template,
                                                spawn_interval,
                                                spawn_delta,
                                                max_spawns,
                                                current_spawns,
                                                active);
                               }),
                           parseByteString(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<int>(),
                           parseByte<int>(),
                           parseByte<bool>())

  DEFAULT_SERIALIZE(string_to_byte(this->entity_template),
                    type_to_byte(this->spawn_interval),
                    type_to_byte(this->spawn_delta),
                    type_to_byte(this->max_spawns),
                    type_to_byte(this->current_spawns),
                    type_to_byte(this->active))

  std::string entity_template;
  double spawn_interval;
  double spawn_delta;
  int max_spawns;
  int current_spawns;
  bool active;
};
