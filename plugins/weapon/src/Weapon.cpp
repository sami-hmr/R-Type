#include <vector>

#include "Weapon.hpp"

#include "WeaponHelpers.hpp"
#include "ecs/EmitEvent.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/components/AnimatedSprite.hpp"
#include "plugin/components/BasicWeapon.hpp"
#include "plugin/components/ChargeWeapon.hpp"
#include "plugin/components/Damage.hpp"
#include "plugin/components/DelayedWeapon.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Facing.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/ScaleModifier.hpp"
#include "plugin/components/Sprite.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/events/AnimationEvents.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/WeaponEvent.hpp"
#include "plugin/events/WeaponSwitchEvent.hpp"

Weapon::Weapon(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("weapon",
              r,
              em,
              l,
              {"moving", "life"},
              {COMP_INIT(BasicWeapon, BasicWeapon, init_basic_weapon),
               COMP_INIT(ChargeWeapon, ChargeWeapon, init_charge_weapon),
               COMP_INIT(DelayedWeapon, DelayedWeapon, init_delayed_weapon)})
    , entity_loader(l)
{
  REGISTER_COMPONENT(BasicWeapon)
  REGISTER_COMPONENT(ChargeWeapon)
  REGISTER_COMPONENT(DelayedWeapon)
  REGISTER_COMPONENT(ScaleModifier)
  SUBSCRIBE_EVENT(FireBullet, {
    std::cout << "WHTF" << std::endl;
    if (this->_registry.get().has_component<ChargeWeapon>(event.entity)) {
      emit_event<StartChargeWeapon>(
          this->_event_manager.get(), "StartChargeWeapon", event.entity);
    } else {
      this->on_fire(this->_registry.get(), event);
    }
  })
  SUBSCRIBE_EVENT(StartChargeWeapon,
                  { this->on_charge_start(this->_registry.get(), event); })
  SUBSCRIBE_EVENT(ReleaseChargeWeapon,
                  { this->on_charge_release(this->_registry.get(), event); })

  SUBSCRIBE_EVENT(WeaponSwitchEvent, {
    if (this->_registry.get().has_component<BasicWeapon>(event.entity)) {
      this->_registry.get().remove_component<BasicWeapon>(event.entity);
    }
    if (this->_registry.get().has_component<ChargeWeapon>(event.entity)) {
      this->_registry.get().remove_component<ChargeWeapon>(event.entity);
    }
    if (this->_registry.get().has_component<DelayedWeapon>(event.entity)) {
      this->_registry.get().remove_component<DelayedWeapon>(event.entity);
    }
    if (event.weapon_type == "ChargeWeapon") {
      init_charge_weapon(event.entity, event.params);
    } else if (event.weapon_type == "BasicWeapon") {
      init_basic_weapon(event.entity, event.params);
    } else if (event.weapon_type == "DelayedWeapon") {
      init_delayed_weapon(event.entity, event.params);
    }
  })

  _registry.get().add_system([this](Registry& r)
                             { this->basic_weapon_system(r.clock().now()); });
  _registry.get().add_system([this](Registry& r)
                             { this->charge_weapon_system(r.clock().now()); });
  _registry.get().add_system([this](Registry& r)
                             { this->delayed_weapon_system(r.clock().now()); });
  _registry.get().add_system([this](Registry&)
                             { this->apply_scale_modifiers(); });
}

void Weapon::basic_weapon_system(
    std::chrono::high_resolution_clock::time_point now)
{
  handle_reload_system<BasicWeapon>(now);
}

void Weapon::on_fire(Registry& r, const FireBullet& e)
{
  fire_basic(r, e);
  fire_delayed(r, e);
}

void Weapon::apply_scale_modifiers()
{
  for (auto&& [entity, modifier, sprite] :
       ZipperIndex<ScaleModifier, Sprite>(this->_registry.get()))
  {
    if (!modifier.applied) {
      sprite.scale = sprite.scale * modifier.scale_multiplier;

      if (modifier.scale_damage
          && this->_registry.get().has_component<Damage>(entity))
      {
        auto& damage = *this->_registry.get().get_components<Damage>()[entity];
        damage.amount =
            static_cast<int>(damage.amount * modifier.scale_multiplier);
      }

      modifier.applied = true;
    }
  }

  for (auto&& [entity, modifier, animated_sprite] :
       ZipperIndex<ScaleModifier, AnimatedSprite>(this->_registry.get()))
  {
    if (!modifier.applied) {
      auto anim_it =
          animated_sprite.animations.find(animated_sprite.current_animation);
      if (anim_it != animated_sprite.animations.end()) {
        anim_it->second.sprite_size =
            anim_it->second.sprite_size * modifier.scale_multiplier;
      }

      if (modifier.scale_damage
          && this->_registry.get().has_component<Damage>(entity))
      {
        auto& damage = *this->_registry.get().get_components<Damage>()[entity];
        damage.amount =
            static_cast<int>(damage.amount * modifier.scale_multiplier);
      }

      modifier.applied = true;
    }
  }
}

extern "C"
{
PLUGIN_EXPORT void* entry_point(Registry& r, EventManager& em, EntityLoader& l)
{
  return new Weapon(r, em, l);
}
}
