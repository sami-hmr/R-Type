#pragma once

#include <chrono>
#include <string>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "ecs/Registry.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/BaseWeapon.hpp"
#include "plugin/events/EventMacros.hpp"

struct BasicWeapon : public BaseWeapon
{
  BasicWeapon() = default;

  BasicWeapon(std::string bullet_type,
              int magazine_size,
              int magazine_nb,
              double reload_time,
              double cooldown,
              double offset_x = 0.0,
              double offset_y = 0.0,
              std::string attack_animation = "")
      : BaseWeapon(std::move(bullet_type),
                   magazine_size,
                   magazine_nb,
                   reload_time,
                   cooldown,
                   offset_x,
                   offset_y,
                   std::move(attack_animation))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(BasicWeapon,
                           (
                               [](std::string bullet_type,
                                  int mag_size,
                                  int mag_nb,
                                  double reload_time,
                                  double cooldown,
                                  double offset_x,
                                  double offset_y,
                                  std::string attack_animation)
                               {
                                 return BasicWeapon(bullet_type,
                                                    mag_size,
                                                    mag_nb,
                                                    reload_time,
                                                    cooldown,
                                                    offset_x,
                                                    offset_y,
                                                    attack_animation);
                               }),
                           parseByteString(),
                           parseByte<int>(),
                           parseByte<int>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByteString())
  DEFAULT_SERIALIZE(string_to_byte(this->bullet_type),
                    type_to_byte(this->magazine_size),
                    type_to_byte(this->magazine_nb),
                    type_to_byte(this->reload_time),
                    type_to_byte(this->cooldown),
                    type_to_byte(this->offset_x),
                    type_to_byte(this->offset_y),
                    string_to_byte(this->attack_animation))
  CHANGE_ENTITY_DEFAULT

  HOOKABLE(BasicWeapon,
           HOOK(bullet_type),
           HOOK(magazine_size),
           HOOK(magazine_nb),
           HOOK(remaining_ammo),
           HOOK(remaining_magazine),
           HOOK(reload_time),
           HOOK(reloading),
           HOOK(last_reload_time),
           HOOK(cooldown),
           HOOK(offset_x),
           HOOK(offset_y),
           HOOK(attack_animation))

  /**
   * Update weapon state and check if it can fire
   * @param now Current time point
   * @return true if weapon can fire, false otherwise
   */
  bool update_basic_weapon(std::chrono::high_resolution_clock::time_point now)
  {
    return update_weapon(now);
  }
};
