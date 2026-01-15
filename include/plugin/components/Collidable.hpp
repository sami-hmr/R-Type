#pragma once

#include <string>
#include <vector>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

enum class CollisionType : std::uint8_t
{
  Bounce,
  Push,
  Solid,
  Trigger
};

struct Collidable
{
  Collidable() = default;

  Collidable(Vector2D const& size,
             CollisionType type = CollisionType::Solid,
             bool active = true)
      : size(size)
      , collision_type(type)
      , is_active(active)
  {
  }

  Collidable(Vector2D const& size,
             CollisionType type,
             bool active,
             const std::vector<std::string>& exclude)
      : size(size)
      , collision_type(type)
      , is_active(active)
      , exclude_entities(exclude)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      Collidable,
      (
          [](Vector2D const& vector,
             CollisionType collision_type,
             bool active,
             std::vector<std::vector<char>> const& excludes)
          {
            std::vector<std::string> r;
            r.reserve(excludes.size());
            for (auto const& it : excludes) {
              r.emplace_back(it.begin(), it.end());
            }
            return Collidable(vector, collision_type, active, r);
          }),
      parseVector2D(),
      parseByte<CollisionType>(),
      parseByte<bool>(),
      parseByteArray(parseByteArray(parseAnyChar())))
      
  DEFAULT_SERIALIZE(vector2DToByte(this->size),
                    type_to_byte(this->collision_type),
                    type_to_byte(this->is_active),
                    vector_to_byte<std::string>(this->exclude_entities,
                                                string_to_byte))

  CHANGE_ENTITY_DEFAULT

  Vector2D size;
  CollisionType collision_type = CollisionType::Solid;
  bool is_active = true;
  std::vector<std::string> exclude_entities;

  HOOKABLE(Collidable,
           HOOK(size),
           HOOK(collision_type),
           HOOK(is_active),
           HOOK(exclude_entities))
};
