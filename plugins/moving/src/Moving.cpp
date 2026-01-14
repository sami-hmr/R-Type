#include <algorithm>
#include <cmath>
#include <iostream>

#include "Moving.hpp"

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
#include "plugin/components/BasicMap.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Facing.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/RaycastingCamera.hpp"
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
               COMP_INIT(Facing, Facing, init_facing)})
{
  REGISTER_COMPONENT(Position)
  REGISTER_COMPONENT(Direction)
  REGISTER_COMPONENT(Speed)
  REGISTER_COMPONENT(Facing)

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

  auto& raycasting_cameras = reg.get_components<RaycastingCamera>();
  auto& basic_maps = reg.get_components<BasicMap>();

  for (auto&& [index, position, direction, speed] :
       ZipperIndex<Position, Direction, Speed>(reg))
  {
    Vector2D real_direction = direction.direction;
    bool use_grid_collision = false;

    if (index < raycasting_cameras.size()
        && raycasting_cameras[index].has_value())
    {
      double cam_angle = raycasting_cameras[index]->angle;
      real_direction.rotate_radians(cam_angle);
      use_grid_collision = true;
    }

    Vector2D movement = real_direction.normalize() * speed.speed * dt;

    if (use_grid_collision && movement.length() > 0) {
      constexpr double player_radius = 0.2;
      Vector2D new_pos = position.pos + movement;

      for (auto const& map_opt : basic_maps) {
        if (!map_opt.has_value()) {
          continue;
        }
        auto const& map = map_opt.value();

        if (position.pos.x < 0 || position.pos.x >= map.size.x
            || position.pos.y < 0 || position.pos.y >= map.size.y)
        {
          continue;
        }

        int check_x = static_cast<int>(std::floor(
            new_pos.x + (movement.x > 0 ? player_radius : -player_radius)));
        int current_y = static_cast<int>(std::floor(position.pos.y));
        if (check_x >= 0 && check_x < static_cast<int>(map.size.x)
            && current_y >= 0 && current_y < static_cast<int>(map.size.y)
            && map.data[current_y][check_x] != 0)
        {
          movement.x = 0;
        }

        int current_x = static_cast<int>(std::floor(position.pos.x));
        int check_y = static_cast<int>(std::floor(
            new_pos.y + (movement.y > 0 ? player_radius : -player_radius)));
        if (current_x >= 0 && current_x < static_cast<int>(map.size.x)
            && check_y >= 0 && check_y < static_cast<int>(map.size.y)
            && map.data[check_y][current_x] != 0)
        {
          movement.y = 0;
        }
      }
    }

    position.pos += movement;
    if (movement.length() != 0) {
      this->_event_manager.get().emit<ComponentBuilder>(
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
                                           this->_event_manager.get(),
                                           entity,
                                           values.value(),
                                           z);

  if (!pos_opt.has_value()) {
    std::cerr << "Error creating Position component\n";
    return;
  }
}

void Moving::init_direction(Registry::Entity const& entity, JsonObject& obj)
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

void Moving::init_speed(Registry::Entity const& entity, JsonObject& obj)
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

void Moving::init_facing(Registry::Entity const& entity, JsonObject& obj)
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
