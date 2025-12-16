#include <algorithm>
#include <iostream>

#include "Moving.hpp"

#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"
#include "plugin/events/CollisionEvent.hpp"

Moving::Moving(Registry& r, EntityLoader& l)
    : APlugin("moving",
              r,
              l,
              {},
              {COMP_INIT(Position, Position, init_pos),
               COMP_INIT(Velocity, Velocity, init_velocity)})
{
  REGISTER_COMPONENT(Position)
  REGISTER_COMPONENT(Velocity)

  this->_registry.get().add_system(
      [this](Registry& r) { this->moving_system(r); }, 4);

  SUBSCRIBE_EVENT(UpdateVelocity, {
    std::cout << "VELOCITY\n";
    auto& comp = this->_registry.get().get_components<Velocity>()[event.entity];
    if (!comp) {
      return;
    }
    this->_registry.get().emit<EventBuilder>("UpdateVelocity",
                                             event.to_bytes());
    comp->direction.x = std::min(-1.0, std::max(comp->direction.x + event.x_axis, 1.0));
    comp->direction.y = std::min(-1.0, std::max(comp->direction.y + event.y_axis, 1.0));
  })
}

void Moving::moving_system(Registry& reg)
{
  double dt = reg.clock().delta_seconds();

  for (auto&& [index, position, velocity] :
       ZipperIndex<Position, Velocity>(reg))
  {
    Vector2D movement = (velocity.direction * dt).normalize() * velocity.speed;
    position.pos += movement;
    if (movement.length() != 0) {
      reg.emit<ComponentBuilder>(
          index, reg.get_component_key<Position>(), position.to_bytes());
    }
  }
}

void Moving::init_pos(Registry::Entity const& entity, JsonObject& obj)
{
  auto values =
      get_value<Position, Vector2D>(this->_registry.get(), obj, entity, "pos");

  if (!values.has_value()) {
    std::cerr << "Error creating Position component\n";
    return;
  }
  int z = 1;
  if (obj.contains("z")) {
    auto const& z_value =
        get_value<Position, int>(this->_registry.get(), obj, entity, "z");
    if (z_value) {
      z = z_value.value();
    } else {
      std::cerr << "Error loading Position component: unexpected value type "
                   "(expected z: int)\n";
    }
  }
  auto& pos_opt = init_component<Position>(this->_registry.get(),
      entity, values.value(), z);

  if (!pos_opt.has_value()) {
    std::cerr << "Error creating Position component\n";
    return;
  }
}

void Moving::init_velocity(Registry::Entity const& entity, JsonObject& obj)
{
  auto speed = get_value<Velocity, Vector2D>(
      this->_registry.get(), obj, entity, "speed");
  auto dir = get_value<Velocity, Vector2D>(
      this->_registry.get(), obj, entity, "direction");

  if (!speed || !dir) {
    std::cerr << "Error loading velocity component: missing speed or direction "
                 "in JsonObject\n";
    return;
  }

  auto& vel_opt = init_component<Velocity>(this->_registry.get(),
      entity, speed.value(), dir.value());

  if (!vel_opt.has_value()) {
    std::cerr << "Error creating Velocity component\n";
    return;
  }
}

extern "C"
{
void* entry_point(Registry& r, EntityLoader& e)
{
  return new Moving(r, e);
}
}
