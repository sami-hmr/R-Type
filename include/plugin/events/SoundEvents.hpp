#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>

#include "EventMacros.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Hooks.hpp"

struct PlaySoundEvent
{
  Ecs::Entity entity;
  std::string name;
  double volume;
  double pitch;
  bool loop;

  PlaySoundEvent(Ecs::Entity entity,
                 std::string name,
                 double volume = 100.0,
                 double pitch = 1.0,
                 bool loop = false)
      : entity(entity)
      , name(std::move(name))
      , volume(volume)
      , pitch(pitch)
      , loop(loop)
  {
  }

  PlaySoundEvent(Registry& r,
                 JsonObject const& e,
                 std::optional<Ecs::Entity> entity)
      : entity(get_value_copy<Ecs::Entity>(r, e, "entity", entity).value())
      , name(get_value_copy<std::string>(r, e, "name", entity).value())
      , volume(get_value_copy<double>(r, e, "volume", entity).value())
      , pitch(get_value_copy<double>(r, e, "pitch", entity).value())
      , loop(get_value_copy<bool>(r, e, "loop", entity).value())
  {
  }

  CHANGE_ENTITY(result.entity = map.at(entity);)

  DEFAULT_BYTE_CONSTRUCTOR(
      PlaySoundEvent,
      ([](Ecs::Entity e, std::string n, double v, double p, bool l)
       { return PlaySoundEvent(e, std::move(n), v, p, l); }),
      parseByte<Ecs::Entity>(),
      parseByteString(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<bool>())

  DEFAULT_SERIALIZE(type_to_byte(this->entity),
                    string_to_byte(this->name),
                    type_to_byte(this->volume),
                    type_to_byte(this->pitch),
                    type_to_byte(this->loop))
};
