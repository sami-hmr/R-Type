#pragma once

#include <chrono>
#include <optional>
#include <string>

#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/BaseWeapon.hpp"
#include "plugin/events/EventMacros.hpp"

struct ChargeWeapon : public BaseWeapon
{
  double charge_time;
  double max_scale;
  double min_charge_threshold;
  bool scale_damage;
  std::string charge_indicator;

  // Charging state
  bool is_charging = false;
  double current_charge_level = 0.0;
  std::chrono::high_resolution_clock::time_point charge_start_time;
  std::optional<Ecs::Entity> charge_indicator_entity = std::nullopt;
  Vector2D charge_indicator_base_scale = Vector2D(1.0, 1.0);

  ChargeWeapon() = default;

  ChargeWeapon(std::string bullet_type,
               int magazine_size,
               int magazine_nb,
               double reload_time,
               double cooldown,
               double charge_time,
               double max_scale,
               double min_charge_threshold,
               bool scale_damage,
               double offset_x = 0.0,
               double offset_y = 0.0,
               std::string attack_animation = "",
               std::string charge_indicator = "")
      : BaseWeapon(std::move(bullet_type),
                   magazine_size,
                   magazine_nb,
                   reload_time,
                   cooldown,
                   offset_x,
                   offset_y,
                   std::move(attack_animation))
      , charge_time(charge_time)
      , max_scale(max_scale)
      , min_charge_threshold(min_charge_threshold)
      , scale_damage(scale_damage)
      , charge_indicator(std::move(charge_indicator))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      ChargeWeapon,
      (
          [](std::string bullet_type,
             int mag_size,
             int mag_nb,
             double reload_time,
             double cooldown,
             double offset_x,
             double offset_y,
             double charge_time,
             double max_scale,
             double min_charge_threshold,
             bool scale_damage,
             std::string attack_animation,
             std::string charge_indicator,
             int remaining_ammo,
             int remaining_magazine,
             bool reloading,
             bool is_charging,
             std::optional<Ecs::Entity> charge_indicator_entity,
             double current_charge_level,
             Vector2D charge_indicator_base_scale)
          {
            ChargeWeapon weapon(bullet_type,
                                mag_size,
                                mag_nb,
                                reload_time,
                                cooldown,
                                charge_time,
                                max_scale,
                                min_charge_threshold,
                                scale_damage,
                                offset_x,
                                offset_y,
                                attack_animation,
                                charge_indicator);
            weapon.remaining_ammo = remaining_ammo;
            weapon.remaining_magazine = remaining_magazine;
            weapon.reloading = reloading;
            weapon.is_charging = is_charging;
            weapon.charge_indicator_entity = charge_indicator_entity;
            weapon.current_charge_level = current_charge_level;
            weapon.charge_indicator_base_scale = charge_indicator_base_scale;
            return weapon;
          }),
      parseByteString(),
      parseByte<int>(),
      parseByte<int>(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<double>(),
      parseByte<bool>(),
      parseByteString(),
      parseByteString(),
      parseByte<int>(),
      parseByte<int>(),
      parseByte<bool>(),
      parseByte<bool>(),
      parseByteOptional(parseByte<Ecs::Entity>()),
      parseByte<double>(),
      parseByte<Vector2D>())

  DEFAULT_SERIALIZE(string_to_byte(this->bullet_type),
                    type_to_byte(this->magazine_size),
                    type_to_byte(this->magazine_nb),
                    type_to_byte(this->reload_time),
                    type_to_byte(this->cooldown),
                    type_to_byte(this->offset_x),
                    type_to_byte(this->offset_y),
                    type_to_byte(this->charge_time),
                    type_to_byte(this->max_scale),
                    type_to_byte(this->min_charge_threshold),
                    type_to_byte(this->scale_damage),
                    string_to_byte(this->attack_animation),
                    string_to_byte(this->charge_indicator),
                    type_to_byte(this->remaining_ammo),
                    type_to_byte(this->remaining_magazine),
                    type_to_byte(this->reloading),
                    type_to_byte(this->is_charging),
                    optional_to_byte<Ecs::Entity>(
                        this->charge_indicator_entity,
                        std::function<ByteArray(Ecs::Entity const&)>(
                            [](Ecs::Entity const& e)
                            { return type_to_byte(e); })),
                    type_to_byte(this->current_charge_level),
                    type_to_byte(this->charge_indicator_base_scale))
  CHANGE_ENTITY(
      result.charge_indicator_entity = this->charge_indicator_entity.has_value()
          ? std::make_optional<Ecs::Entity>(
                map.at(this->charge_indicator_entity.value()))
          : std ::nullopt)

  HOOKABLE(ChargeWeapon,
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
           HOOK(charge_time),
           HOOK(max_scale),
           HOOK(min_charge_threshold),
           HOOK(scale_damage),
           HOOK(attack_animation),
           HOOK(charge_indicator))

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
