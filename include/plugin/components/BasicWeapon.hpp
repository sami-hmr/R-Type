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

  BasicWeapon(std::string bullet_type,
              int magazine_size,
              int magazine_nb,
              double reload_time,
              double cooldown)
      : bullet_type(std::move(bullet_type))
      , magazine_size(magazine_size)
      , magazine_nb(magazine_nb)
      , remaining_ammo(magazine_size)
      , remaining_magazine(magazine_nb)
      , reload_time(reload_time)
      , cooldown(cooldown)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(BasicWeapon,
                           (
                               [](std::string bullet_type,
                                  int mag_size,
                                  int mag_nb,
                                  double reload_time,
                                  double cooldown)
                               {
                                 return BasicWeapon(bullet_type,
                                                    mag_size,
                                                    mag_nb,
                                                    reload_time,
                                                    cooldown);
                               }),
                           parseByteString(),
                           parseByte<int>(),
                           parseByte<int>(),
                           parseByte<double>(),
                           parseByte<double>())
  DEFAULT_SERIALIZE(string_to_byte(this->bullet_type),
                    type_to_byte(this->magazine_size),
                    type_to_byte(this->magazine_nb),
                    type_to_byte(this->reload_time),
                    type_to_byte(this->cooldown))
  CHANGE_ENTITY_DEFAULT

  std::string bullet_type;
  int magazine_size;
  int magazine_nb;
  int remaining_ammo;
  int remaining_magazine;
  double reload_time;
  double cooldown;
  bool reloading = false;
  std::chrono::high_resolution_clock::time_point last_shot_time;
  std::chrono::high_resolution_clock::time_point last_reload_time;

  HOOKABLE(BasicWeapon,
           HOOK(bullet_type),
           HOOK(magazine_size),
           HOOK(magazine_nb),
           HOOK(remaining_ammo),
           HOOK(remaining_magazine),
           HOOK(reload_time),
           HOOK(reloading),
           HOOK(last_reload_time),
           HOOK(cooldown))

  bool update_basic_weapon(std::chrono::high_resolution_clock::time_point now);
};
