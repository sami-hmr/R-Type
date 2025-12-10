#include <iostream>

#include "Moving.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"

Moving::Moving(Registry& r, EntityLoader& l)
    : APlugin(r,
              l,
              {},
              {COMP_INIT(Position, Position, init_pos),
               COMP_INIT(Velocity, Velocity, init_velocity)})
{
  this->_registry.get().register_component<Position>("moving:Position");
  this->_registry.get().register_component<Velocity>("moving:Velocity");

  this->_registry.get().add_system<Position, Velocity>(
      [this](Registry& r,
             SparseArray<Position>& pos,
             const SparseArray<Velocity>& vel)
      { this->moving_system(r, pos, vel); },
      4);
}

void Moving::moving_system(Registry& reg,
                           SparseArray<Position>& positions,
                           const SparseArray<Velocity>& velocities)
{
  double dt = reg.clock().delta_seconds();

  for (auto&& [position, velocity] : Zipper(positions, velocities)) {
    Vector2D movement = (velocity.direction * dt).normalize() * velocity.speed;
    position.pos += movement;
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
  auto& pos_opt = this->_registry.get().emplace_component<Position>(
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

  auto& vel_opt = this->_registry.get().emplace_component<Velocity>(
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
