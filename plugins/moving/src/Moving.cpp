#include <iostream>

#include "Moving.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"

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
    if (velocity.dir_x != 0.0) {
      position.x += velocity.speed_x * velocity.dir_x * dt;
    }
    if (velocity.dir_y != 0.0) {
      position.y += velocity.speed_y * velocity.dir_y * dt;
    }
  }
}

void Moving::init_pos(Registery::Entity const& entity, JsonObject const& obj)
{
  auto const& x = get_value<double>(this->_registery.get(), obj, "x");
  auto const& y = get_value<double>(this->_registery.get(), obj, "y");

  if (!x || !y) {
    std::cerr << "Error loading Position component: unexpected value type "
                 "(expected x: double and y: double)\n";
    return;
  }

  this->_registery.get().emplace_component<Position>(
      entity, x.value(), y.value());
}

void Moving::init_velocity(Registery::Entity const& entity,
                           JsonObject const& obj)
{
  auto const& speed_x =
      get_value<double>(this->_registery.get(), obj, "speed_x");
  auto const& speed_y =
      get_value<double>(this->_registery.get(), obj, "speed_y");
  auto const& dir_x = get_value<double>(this->_registery.get(), obj, "dir_x");
  auto const& dir_y = get_value<double>(this->_registery.get(), obj, "dir_y");

  if (!speed_x || !speed_y || !dir_x || !dir_y) {
    std::cerr
        << "Error loading velocity component: unexpected value type (speed_x: "
           "double, speed_y: double, dir_x: double, dir_y: double)\n";
    return;
  }

  this->_registery.get().emplace_component<Velocity>(
      entity, speed_x.value(), speed_y.value(), dir_x.value(), dir_y.value());
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new Moving(r, e);
}
}
