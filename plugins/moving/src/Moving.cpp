#include <iostream>

#include "Moving.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"
#include "plugin/events/Events.hpp"

Moving::Moving(Registery& r, EntityLoader& l)
    : APlugin(r,
              l,
              {},
              {COMP_INIT(Position, Position, init_pos),
               COMP_INIT(Velocity, Velocity, init_velocity)})
{
  this->_registery.get().register_component<Position>("moving:Position");
  this->_registery.get().register_component<Velocity>("moving:Velocity");

  this->_registery.get().add_system<Position, Velocity>(
      [this](Registery& r,
             SparseArray<Position>& pos,
             const SparseArray<Velocity>& vel)
      { this->moving_system(r, pos, vel); },
      4);
}

void Moving::moving_system(Registery& reg,
                           SparseArray<Position>& positions,
                           const SparseArray<Velocity>& velocities)
{
  double dt = reg.clock().delta_seconds();

  for (auto&& [position, velocity] : Zipper(positions, velocities)) {
    Vector2D movement = velocity.direction * dt;
    position.pos += movement.normalize() * velocity.speed;
  }
}

void Moving::init_pos(Registery::Entity const& entity, JsonObject& obj)
{
  auto values =
      get_value<Position, Vector2D>(this->_registery.get(), obj, entity, "pos");

  if (!values.has_value()) {
    std::cerr << "Error creating Position component\n";
    return;
  }

  auto& pos_opt = this->_registery.get().emplace_component<Position>(
      entity, values.value());

  if (!pos_opt.has_value()) {
    std::cerr << "Error creating Position component\n";
    return;
  }
}

void Moving::init_velocity(Registery::Entity const& entity, JsonObject& obj)
{
  auto speed = get_value<Position, Vector2D>(
      this->_registery.get(), obj, entity, "speed");
  auto dir = get_value<Position, Vector2D>(
      this->_registery.get(), obj, entity, "direction");

  if (!speed || !dir) {
    std::cerr << "Error loading velocity component: missing speed or direction "
                 "in JsonObject\n";
    return;
  }

  auto& vel_opt = this->_registery.get().emplace_component<Velocity>(
      entity, speed.value(), dir.value());

  if (!vel_opt.has_value()) {
    std::cerr << "Error creating Velocity component\n";
    return;
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new Moving(r, e);
}
}
