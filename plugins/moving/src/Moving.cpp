#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>

#include "Moving.hpp"

#include "EntityExpose.hpp"
#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Facing.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Speed.hpp"
#include "plugin/events/CollisionEvent.hpp"

Moving::Moving(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("moving",
              r,
              em,
              l,
              {},
              {COMP_INIT(Position, Position, init_pos),
               COMP_INIT(Direction, Direction, init_direction),
               COMP_INIT(Speed, Speed, init_speed),
               COMP_INIT(Facing, Facing, init_facing),
               COMP_INIT(IdStorage, IdStorage, init_id)})
{
  REGISTER_COMPONENT(Position)
  REGISTER_COMPONENT(Direction)
  REGISTER_COMPONENT(Speed)
  REGISTER_COMPONENT(Facing)
  REGISTER_COMPONENT(IdStorage)
  this->_registry.get().add_system(
      [this](Registry& r) { this->moving_system(r); }, 4);

  SUBSCRIBE_EVENT(UpdateDirection, {
    if (!this->_registry.get().has_component<Direction>(event.entity)) {
      return false;
    }
    auto& comp =
        this->_registry.get().get_components<Direction>()[event.entity];
    comp->direction.x =
        std::max(-1.0, std::min(comp->direction.x + event.x_axis, 1.0));
    comp->direction.y =
        std::max(-1.0, std::min(comp->direction.y + event.y_axis, 1.0));
  })
}

void Moving::moving_system(Registry& reg)
{
  double dt = reg.clock().delta_seconds();

  for (auto&& [index, position, direction, speed] :
       ZipperIndex<Position, Direction, Speed>(reg))
  {
    Vector2D movement = (direction.direction).normalize() * speed.speed * dt;
    position.pos += movement;
    if (movement.length() != 0) {
      this->_event_manager.get().emit<ComponentBuilder>(
          index, reg.get_component_key<Position>(), position.to_bytes());
    }
  }
}

void Moving::init_id(Ecs::Entity const& entity, JsonObject& obj)
{
  std::string ctx;
  auto id = get_value<IdStorage, std::size_t>(
      this->_registry.get(), obj, entity, "id");
  if (obj.contains("context")) {
    ctx = get_value<IdStorage, std::string>(
              this->_registry.get(), obj, entity, "context")
              .value();
  }
  auto& pos_opt = init_component<IdStorage>(this->_registry.get(),
                                            this->_event_manager.get(),
                                            entity,
                                            id.value(),
                                            ctx);
  if (!pos_opt.has_value()) {
    std::cerr << "Error creating IdStorage component\n";
    return;
  }
}

void Moving::init_pos(Ecs::Entity const& entity, JsonObject& obj)
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
                                           this->_event_manager.get(),
                                           entity,
                                           values.value(),
                                           z);

  if (!pos_opt.has_value()) {
    std::cerr << "Error creating Position component\n";
    return;
  }
}

void Moving::init_direction(Ecs::Entity const& entity, JsonObject& obj)
{
  auto dir = get_value<Direction, Vector2D>(
      this->_registry.get(), obj, entity, "direction");

  if (!dir) {
    std::cerr << "Error loading Direction component: missing direction "
                 "in JsonObject\n";
    return;
  }

  auto& vel_opt = init_component<Direction>(
      this->_registry.get(), this->_event_manager.get(), entity, dir.value());

  if (!vel_opt.has_value()) {
    std::cerr << "Error creating Direction component\n";
    return;
  }
}

void Moving::init_speed(Ecs::Entity const& entity, JsonObject& obj)
{
  auto speed =
      get_value<Speed, Vector2D>(this->_registry.get(), obj, entity, "speed");

  if (!speed) {
    std::cerr
        << "Error loading Speed component: missing speed " "in JsonObject\n";
    return;
  }

  auto& vel_opt = init_component<Speed>(
      this->_registry.get(), this->_event_manager.get(), entity, speed.value());

  if (!vel_opt.has_value()) {
    std::cerr << "Error creating Speed component\n";
    return;
  }
}

void Moving::init_facing(Ecs::Entity const& entity, JsonObject& obj)
{
  auto dir = get_value<Facing, Vector2D>(
      this->_registry.get(), obj, entity, "direction");

  if (!dir) {
    std::cerr << "Error loading Facing component: missing direction "
                 "in JsonObject\n";
    return;
  }
  auto& facing_opt =
      this->_registry.get().emplace_component<Facing>(entity, dir.value());

  if (!facing_opt.has_value()) {
    std::cerr << "Error creating Facing component\n";
    return;
  }
}

extern "C"
{
void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new Moving(r, em, e);
}
}
