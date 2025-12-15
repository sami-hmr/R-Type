#pragma once

#include <cstdint>

#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

enum Enemy_type : std::uint8_t
{
  BOSS = 0,
  SUBBOSS,
  MASTER,
  ELITE,
  COMMON,
  NOOB
};

struct Enemy
{
  Enemy_type type;

  Enemy(Enemy_type type)
      : type(type)
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(Enemy,
                           ([](Enemy_type type) { return Enemy(type); }),
                           parseByte<Enemy_type>())
  DEFAULT_SERIALIZE(type_to_byte(this->type))

  HOOKABLE(Enemy, HOOK(type))
};
