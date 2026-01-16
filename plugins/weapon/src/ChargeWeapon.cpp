#include "plugin/components/ChargeWeapon.hpp"

#include "Weapon.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Hooks.hpp"

void Weapon::init_charge_weapon(Registry::Entity const& entity,
                                JsonObject const& obj)
{
  auto const& bullet_type = get_value<ChargeWeapon, std::string>(
      this->_registry.get(), obj, entity, "bullet_type");
  if (!bullet_type) {
    std::cerr << "Error loading ChargeWeapon component: unexpected value type "
                     "(bullet_type: string)\n";
    return;
  }

  auto const& magazine_size = get_value<ChargeWeapon, int>(
      this->_registry.get(), obj, entity, "magazine_size");
  if (!magazine_size) {
    std::cerr << "Error loading ChargeWeapon component: unexpected value type "
                     "(magazine_size: int)\n";
    return;
  }

  auto const& magazine_nb = get_value<ChargeWeapon, int>(
      this->_registry.get(), obj, entity, "magazine_nb");
  if (!magazine_nb) {
    std::cerr << "Error loading ChargeWeapon component: unexpected value type "
                     "(magazine_nb: int)\n";
    return;
  }

  auto const& reload_time = get_value<ChargeWeapon, double>(
      this->_registry.get(), obj, entity, "reload_time");
  if (!reload_time) {
    std::cerr << "Error loading ChargeWeapon component: unexpected value type "
                     "(reload_time: double)\n";
    return;
  }

  auto const& cooldown = get_value<ChargeWeapon, double>(
      this->_registry.get(), obj, entity, "cooldown");
  if (!cooldown) {
    std::cerr << "Error loading ChargeWeapon component: unexpected value type "
                     "(cooldown: double)\n";
    return;
  }

  auto const& charge_time = get_value<ChargeWeapon, double>(
      this->_registry.get(), obj, entity, "charge_time");
  if (!charge_time) {
    std::cerr << "Error loading ChargeWeapon component: unexpected value type "
                     "(charge_time: double)\n";
    return;
  }

  auto const& max_scale = get_value<ChargeWeapon, double>(
      this->_registry.get(), obj, entity, "max_scale");
  if (!max_scale) {
    std::cerr << "Error loading ChargeWeapon component: unexpected value type "
                     "(max_scale: double)\n";
    return;
  }

  auto const& min_charge_threshold = get_value<ChargeWeapon, double>(
      this->_registry.get(), obj, entity, "min_charge_threshold");
  if (!min_charge_threshold) {
    std::cerr << "Error loading ChargeWeapon component: unexpected value type "
                     "(min_charge_threshold: double)\n";
    return;
  }

  auto const& scale_damage = get_value<ChargeWeapon, bool>(
      this->_registry.get(), obj, entity, "scale_damage");
  if (!scale_damage) {
    std::cerr << "Error loading ChargeWeapon component: unexpected value type "
                     "(scale_damage: bool)\n";
    return;
  }

  auto const& attack_animation = get_value<ChargeWeapon, std::string>(
      this->_registry.get(), obj, entity, "attack_animation");
  std::string attack_anim_name =
      attack_animation ? attack_animation.value() : "";

  auto const& charge_indicator = get_value<ChargeWeapon, std::string>(
      this->_registry.get(), obj, entity, "charge_indicator");
  std::string charge_indicator_name =
      charge_indicator ? charge_indicator.value() : "";

  init_component<ChargeWeapon>(this->_registry.get(),
                               this->_event_manager.get(),
                               entity,
                               bullet_type.value(),
                               magazine_size.value(),
                               magazine_nb.value(),
                               reload_time.value(),
                               cooldown.value(),
                               charge_time.value(),
                               max_scale.value(),
                               min_charge_threshold.value(),
                               scale_damage.value(),
                               attack_anim_name,
                               charge_indicator_name);
}
