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

  BasicWeapon(std::string bullet_type, int magazine_size, int magazine_nb, double reload_time)
      : bullet_type(std::move(bullet_type))
      , magazine_size(magazine_size)
      , magazine_nb(magazine_nb)
      , remaining_ammo(magazine_size)
      , remaining_magazine(magazine_nb)
      , reload_time(reload_time)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      BasicWeapon,
      ([](std::string bullet_type, int mag_size, int mag_nb, double reload_time)
       {
         return BasicWeapon(
             bullet_type,
             mag_size,
             mag_nb,
             reload_time);
       }),
      parseByteString(),
      parseByte<int>(),
      parseByte<int>(),
      parseByte<double>())
  DEFAULT_SERIALIZE(string_to_byte(this->bullet_type),
                    type_to_byte(this->magazine_size),
                    type_to_byte(this->magazine_nb),
                    type_to_byte(this->reload_time))

  CHANGE_ENTITY_DEFAULT

  std::string bullet_type;
  int magazine_size;
  int magazine_nb;
  int remaining_ammo;
  int remaining_magazine;
  double reload_time;
  bool reloading = false;
  std::chrono::high_resolution_clock::time_point last_reload_time;

  HOOKABLE(BasicWeapon, HOOK(bullet_type), HOOK(magazine_size), HOOK(magazine_nb), HOOK(remaining_ammo), HOOK(remaining_magazine), HOOK(reload_time),
           HOOK(reloading), HOOK(last_reload_time))

  void update_basic_weapon();
};
