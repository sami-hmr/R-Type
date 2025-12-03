#include <iostream>
#include "Moving.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Events.hpp"

Moving::Moving(Registery& r, EntityLoader& l)
    : APlugin(
          r,
          l,
          {},
          {COMP_INIT(Position, init_pos), COMP_INIT(Velocity, init_velocity)})
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

void Moving::init_pos(Registery::Entity const& entity,
                      JsonVariant const& config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    Vector2D pos(obj);
    this->_registery.get().emplace_component<Position>(entity, pos.x, pos.y);
  } catch (std::bad_variant_access const&) {
    LOGGER("SFML",
           LogLevel::ERROR,
           "Error loading Position component: unexpected value type")
  } catch (std::out_of_range const&) {
    LOGGER(
        "SFML",
        LogLevel::ERROR,
        "Error loading Position component: (expected x: double and y: double )")
  }
}

void Moving::init_velocity(Registery::Entity const& entity,
                           JsonVariant const& config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    double speed_x = std::get<double>(obj.at("speed_x").value);
    double speed_y = std::get<double>(obj.at("speed_y").value);
    double dir_x = std::get<double>(obj.at("dir_x").value);
    double dir_y = std::get<double>(obj.at("dir_y").value);

    this->_registery.get().emplace_component<Velocity>(
        entity, speed_x, speed_y, dir_x, dir_y);

  } catch (std::bad_variant_access const&) {
    LOGGER("SFML",
           LogLevel::ERROR,
           "Error loading velocity component: unexpected value type")
  } catch (std::out_of_range const&) {
    LOGGER("SFML",
           LogLevel::ERROR,
           "Error loading velocity component: missing speed_x, speed_y, dir_x "
           "and dir_y in " "JsonObject")
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new Moving(r, e);
}
}
