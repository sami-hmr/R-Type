#include "plugin/components/BasicWeapon.hpp"

#include "Weapon.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Hooks.hpp"

void Weapon::init_basic_weapon(Ecs::Entity const& entity, JsonObject const& obj)
{
  auto const& bullet_type = get_value<BasicWeapon, std::string>(
      this->_registry.get(), obj, entity, "bullet_type");
  if (!bullet_type) {
    std::cerr << "Error loading BasicWeapon component: unexpected value type "
                     "(bullet_type: string)\n";
    return;
  }

  auto const& magazine_size = get_value<BasicWeapon, int>(
      this->_registry.get(), obj, entity, "magazine_size");
  if (!magazine_size) {
    std::cerr << "Error loading BasicWeapon component: unexpected value type "
                     "(magazine_size: int)\n";
    return;
  }

  auto const& magazine_nb = get_value<BasicWeapon, int>(
      this->_registry.get(), obj, entity, "magazine_nb");
  if (!magazine_nb) {
    std::cerr << "Error loading BasicWeapon component: unexpected value type "
                     "(magazine_nb: int)\n";
    return;
  }
  auto const& reload_time = get_value<BasicWeapon, double>(
      this->_registry.get(), obj, entity, "reload_time");
  if (!reload_time) {
    std::cerr << "Error loading BasicWeapon component: unexpected value type "
                     "(reload_time: double)\n";
    return;
  }
  auto const& cooldown = get_value<BasicWeapon, double>(
      this->_registry.get(), obj, entity, "cooldown");
  if (!cooldown) {
    std::cerr << "Error loading BasicWeapon component: unexpected value type "
                     "(cooldown: double)\n";
    return;
  }

  auto const& attack_animation = get_value<BasicWeapon, std::string>(
      this->_registry.get(), obj, entity, "attack_animation");
  std::string attack_anim_name =
      attack_animation ? attack_animation.value() : "";

  auto const& offset_x = get_value<BasicWeapon, double>(
      this->_registry.get(), obj, entity, "offset_x");
  double offset_x_value = offset_x ? offset_x.value() : 0.0;

  auto const& offset_y = get_value<BasicWeapon, double>(
      this->_registry.get(), obj, entity, "offset_y");
  double offset_y_value = offset_y ? offset_y.value() : 0.0;

  init_component<BasicWeapon>(this->_registry.get(),
                              this->_event_manager.get(),
                              entity,
                              bullet_type.value(),
                              magazine_size.value(),
                              magazine_nb.value(),
                              reload_time.value(),
                              cooldown.value(),
                              offset_x_value,
                              offset_y_value,
                              attack_anim_name);
}
