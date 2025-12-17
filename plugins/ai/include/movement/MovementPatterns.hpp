#pragma once

#include <cmath>

#include "MovementPattern.hpp"
#include "NetworkShared.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Follower.hpp"
#include "plugin/components/InteractionZone.hpp"
#include "plugin/components/MovementBehavior.hpp"
#include "plugin/components/Speed.hpp"

class StraightPattern : public MovementPattern
{
public:
  static constexpr double DIRECTION_TOLERANCE = 0.1;

  void update(Registry::Entity entity,
              Registry& registry,
              MovementBehavior& /*behavior*/,
              Position& /*pos*/,
              Direction& direction,
              Speed& /*speed*/,
              double /*dt*/) override
  {
    Vector2D new_direction(-1.0, 0.0);
    new_direction = new_direction.normalize();

    Vector2D direction_diff = new_direction - direction.direction;
    double direction_change = direction_diff.length();
    if (direction_change > DIRECTION_TOLERANCE) {
      direction.direction = new_direction;
      registry.emit<ComponentBuilder>(entity,
                                      registry.get_component_key<Direction>(),
                                      direction.to_bytes());
    }
  }
};

class WavePattern : public MovementPattern
{
public:
  static constexpr double DIRECTION_TOLERANCE = 0.1;

  void update(Registry::Entity entity,
              Registry& registry,
              MovementBehavior& behavior,
              Position& /*pos*/,
              Direction& direction,
              Speed& /*speed*/,
              double dt) override
  {
    behavior.movement_delta += dt;
    registry.emit<ComponentBuilder>(
        entity,
        registry.get_component_key<MovementBehavior>(),
        behavior.to_bytes());

    double amplitude = 0.7;
    double frequency = 2.0;

    Vector2D new_direction;
    new_direction.x = -1.0;
    new_direction.y = std::sin(behavior.movement_delta * frequency) * amplitude;
    new_direction = new_direction.normalize();

    Vector2D direction_diff = new_direction - direction.direction;
    double direction_change = direction_diff.length();
    if (direction_change > DIRECTION_TOLERANCE) {
      direction.direction = new_direction;
      registry.emit<ComponentBuilder>(entity,
                                      registry.get_component_key<Direction>(),
                                      direction.to_bytes());
    }
  }
};

class ZigzagPattern : public MovementPattern
{
public:
  static constexpr double DIRECTION_TOLERANCE = 0.1;

  void update(Registry::Entity entity,
              Registry& registry,
              MovementBehavior& behavior,
              Position& /*pos*/,
              Direction& direction,
              Speed& /*speed*/,
              double dt) override
  {
    behavior.movement_delta += dt;
    registry.emit<ComponentBuilder>(
        entity,
        registry.get_component_key<MovementBehavior>(),
        behavior.to_bytes());

    double switch_interval = 1.0;
    double angle = 45.0;

    int dir = static_cast<int>(behavior.movement_delta / switch_interval) % 2;
    double rad = angle * M_PI / 180.0;

    Vector2D new_direction;
    new_direction.x = -1.0;
    new_direction.y = dir == 0 ? std::tan(rad) : -std::tan(rad);
    new_direction = new_direction.normalize();

    Vector2D direction_diff = new_direction - direction.direction;
    double direction_change = direction_diff.length();
    if (direction_change > DIRECTION_TOLERANCE) {
      direction.direction = new_direction;
      registry.emit<ComponentBuilder>(entity,
                                      registry.get_component_key<Direction>(),
                                      direction.to_bytes());
    }
  }
};

class CirclePattern : public MovementPattern
{
public:
  static constexpr double DIRECTION_TOLERANCE = 0.1;

  void update(Registry::Entity entity,
              Registry& registry,
              MovementBehavior& behavior,
              Position& pos,
              Direction& direction,
              Speed& /*speed*/,
              double dt) override

  {
    behavior.movement_delta += dt;
    registry.emit<ComponentBuilder>(
        entity,
        registry.get_component_key<MovementBehavior>(),
        behavior.to_bytes());

    double radius = 100.0;
    double angular_speed = 1.5;

    double angle = behavior.movement_delta * angular_speed;

    Vector2D target_pos;
    target_pos.x = std::cos(angle) * radius;
    target_pos.y = std::sin(angle) * radius;

    Vector2D to_target = target_pos - pos.pos;
    if (to_target.length() > 0.1) {
      Vector2D new_direction = to_target.normalize();
      Vector2D direction_diff = new_direction - direction.direction;
      double direction_change = direction_diff.length();

      if (direction_change > DIRECTION_TOLERANCE) {
        direction.direction = new_direction;
        registry.emit<ComponentBuilder>(entity,
                                        registry.get_component_key<Direction>(),
                                        direction.to_bytes());
      }
    }
  }
};

class TurretPattern : public MovementPattern
{
public:
  void update(Registry::Entity entity,
              Registry& registry,
              MovementBehavior& /*behavior*/,
              Position& /*pos*/,
              Direction& direction,
              Speed& speed,
              double /*dt*/) override
  {
    Vector2D default_speed(0.0, 0.0);
    Vector2D default_direction(0.0, 0.0);

    if (speed.speed != default_speed) {
      speed.speed.x = 0.0;
      speed.speed.y = 0.0;
      registry.emit<ComponentBuilder>(
          entity, registry.get_component_key<Speed>(), speed.to_bytes());
    }
    if (direction.direction != default_direction) {
      direction.direction.x = 0.0;
      direction.direction.y = 0.0;
      registry.emit<ComponentBuilder>(entity,
                                      registry.get_component_key<Direction>(),
                                      direction.to_bytes());
    }
  }
};

class FollowTargetPattern : public MovementPattern
{
public:
  void update(Registry::Entity entity,
              Registry& registry,
              MovementBehavior& /*behavior*/,
              Position& /*pos*/,
              Direction& direction,
              Speed& /*speed*/,
              double /*dt*/) override
  {
    if (!registry.has_component<Follower>(entity)) {
      registry.add_component(entity, Follower());
      direction.direction.x = -1.0;
      direction.direction.y = 0.0;
      registry.emit<ComponentBuilder>(entity,
                                      registry.get_component_key<Direction>(),
                                      direction.to_bytes());
    }
    if (!registry.has_component<InteractionZone>(entity)) {
      registry.add_component(entity, InteractionZone(1.5));
      direction.direction.x = -1.0;
      direction.direction.y = 0.0;
      registry.emit<ComponentBuilder>(entity,
                                      registry.get_component_key<Direction>(),
                                      direction.to_bytes());
    }
  }
};
