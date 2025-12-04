#pragma once

#include <any>
#include <concepts>
#include <string>
#include <unordered_map>
#include <vector>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

enum class CollisionType
{
  Solid,
  Trigger
};

struct Collidable
{
  Collidable() = default;

  Collidable(double w,
             double h,
             CollisionType type = CollisionType::Solid,
             bool active = true)
      : width(w)
      , height(h)
      , collision_type(type)
      , is_active(active)
  {
  }

  Collidable(double w,
             double h,
             CollisionType type,
             bool active,
             const std::vector<std::string>& exclude)
      : width(w)
      , height(h)
      , collision_type(type)
      , is_active(active)
      , exclude_entities(exclude)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      Collidable,
      (
          [](double x, double y, std::vector<std::vector<char>> const& excludes)
          {
            std::vector<std::string> r;
            r.reserve(excludes.size());
            for (auto const& it : excludes) {
              r.emplace_back(it.begin(), it.end());
            }
            return Collidable(x, y, CollisionType::Solid, true, r);
          }),
      parseByte<double>(),
      parseByte<double>(),
      parseByteArray(parseByteArray(parseAnyChar())))
  DEFAULT_SERIALIZE(type_to_byte(this->width),
                    type_to_byte(this->height),
                    vector_to_byte<std::string>(this->exclude_entities,
                                                string_to_byte))

  double width = 0.0;
  double height = 0.0;
  CollisionType collision_type = CollisionType::Solid;
  bool is_active = true;
  std::vector<std::string> exclude_entities;

  HOOKABLE(Collidable, HOOK(width), HOOK(height), HOOK(collision_type), HOOK(is_active), HOOK(exclude_entities))
};
