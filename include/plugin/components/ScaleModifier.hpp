#pragma once

#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct ScaleModifier
{
  ScaleModifier() = default;

  ScaleModifier(double scale, bool scale_damage = false)
      : scale_multiplier(scale)
      , scale_damage(scale_damage)
      , applied(false)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(ScaleModifier,
                           ([](double s, bool d, bool /*a*/)
                            { return ScaleModifier(s, d); }),
                           parseByte<double>(),
                           parseByte<bool>(),
                           parseByte<bool>())

  DEFAULT_SERIALIZE(type_to_byte(this->scale_multiplier),
                    type_to_byte(this->scale_damage),
                    type_to_byte(this->applied))

  CHANGE_ENTITY_DEFAULT

  double scale_multiplier;
  bool scale_damage;
  bool applied;  // Track if scaling has been applied

  HOOKABLE(ScaleModifier,
           HOOK(scale_multiplier),
           HOOK(scale_damage),
           HOOK(applied))
};
