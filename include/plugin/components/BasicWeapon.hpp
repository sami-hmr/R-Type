#pragma once

#include <string>
#include <utility>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/IoEvents.hpp"

struct BasicWeapon
{
  BasicWeapon() = default;

  BasicWeapon(std::string bullet_type, int magazine_size, int magazine_nb)
      : bullet_type(std::move(bullet_type))
      , magazine_size(magazine_size)
      , magazine_nb(magazine_nb)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      BasicWeapon,
      ([](std::string bullet_type, int mag_size, int mag_nb)
       {
         return BasicWeapon(
             bullet_type,
             mag_size,
             mag_nb);
       }),
      parseByteString(),
      parseByte<int>(),
      parseByte<int>())
  DEFAULT_SERIALIZE(string_to_byte(this->bullet_type),
                    type_to_byte(this->magazine_size),
                    type_to_byte(this->magazine_nb))

  CHANGE_ENTITY_DEFAULT

  std::string bullet_type;
  int magazine_size;
  int magazine_nb;

  HOOKABLE(BasicWeapon, HOOK(bullet_type), HOOK(magazine_size), HOOK(magazine_nb))
};
