#include "plugin/components/BasicWeapon.hpp"

#include "Weapon.hpp"
#include "ecs/Registry.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/IoEvents.hpp"

void Weapon::init_basic_weapon(Registry::Entity const& entity,
                               JsonObject const& obj)
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

  _registry.get().emplace_component<BasicWeapon>(
      entity, bullet_type.value(), magazine_size.value(), magazine_nb.value());

}
