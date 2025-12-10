#include <iostream>

#include "Moving.hpp"

#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"
#include "plugin/events/CollisionEvent.hpp"

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

  // this->_registry.get().on<ComponentBuilder>("ComponentBuilder", [](ComponentBuilder const &data) {
  //     std::cerr << "ça build un component " << data.id << "\n";
  // });

  // this->_registry.get().on<EventBuilder>("EventBuilder", [](EventBuilder const &data) {
  //     std::cerr << "ça build un event " << data.event_id << "\n";
  // });
  this->_registry.get().on<UpdateVelocity>(
      "UpdateVelocity",
      [this](UpdateVelocity const& data)
      {
        auto& comp =
            this->_registry.get().get_components<Velocity>()[data.entity];
        if (!comp) {
          return;
        }
        this->_registry.get().emit<EventBuilder>("UpdateVelocity",
                                                 data.to_bytes());
        comp->direction.x = data.x_axis;
        comp->direction.y = data.y_axis;
      });
}

void Moving::moving_system(Registry& reg,
                           SparseArray<Position>& positions,
                           const SparseArray<Velocity>& velocities)
{
  double dt = reg.clock().delta_seconds();

  for (auto&& [index, position, velocity] : ZipperIndex(positions, velocities))
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
