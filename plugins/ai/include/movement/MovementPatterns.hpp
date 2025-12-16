#pragma once

#include <cmath>

#include "MovementPattern.hpp"
#include "NetworkShared.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/components/Follower.hpp"
#include "plugin/components/InteractionZone.hpp"
#include "plugin/components/MovementBehavior.hpp"
#include "plugin/components/Velocity.hpp"

class StraightPattern : public MovementPattern
{
public:
  static constexpr double DIRECTION_TOLERANCE = 0.1;

  void update(Registry::Entity entity,
              Registry& registry,
              MovementBehavior& /*behavior*/,
              Position& /*pos*/,
              Velocity& vel,
              double /*dt*/) override
  {
    Vector2D new_direction(-1.0, 0.0);
    new_direction = new_direction.normalize();

    Vector2D direction_diff = new_direction - vel.direction;
    double direction_change = direction_diff.length();
    if (direction_change > DIRECTION_TOLERANCE) {
      vel.direction = new_direction;
      registry.emit<ComponentBuilder>(
          entity, registry.get_component_key<Velocity>(), vel.to_bytes());
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
              Velocity& vel,
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

    Vector2D direction_diff = new_direction - vel.direction;
    double direction_change = direction_diff.length();
    if (direction_change > DIRECTION_TOLERANCE) {
      vel.direction = new_direction;
      registry.emit<ComponentBuilder>(
          entity, registry.get_component_key<Velocity>(), vel.to_bytes());
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
              Velocity& vel,
              double dt) override
  {
    behavior.movement_delta += dt;
    registry.emit<ComponentBuilder>(
      entity,
      registry.get_component_key<MovementBehavior>(),
      behavior.to_bytes());

    double switch_interval = 1.0;
    double angle = 45.0;

    int direction =
        static_cast<int>(behavior.movement_delta / switch_interval) % 2;
    double rad = angle * M_PI / 180.0;

    Vector2D new_direction;
    new_direction.x = -1.0;
    new_direction.y = direction == 0 ? std::tan(rad) : -std::tan(rad);
    new_direction = new_direction.normalize();

    Vector2D direction_diff = new_direction - vel.direction;
    double direction_change = direction_diff.length();
    if (direction_change > DIRECTION_TOLERANCE) {
      vel.direction = new_direction;
      registry.emit<ComponentBuilder>(
          entity, registry.get_component_key<Velocity>(), vel.to_bytes());
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
              Velocity& vel,
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
      Vector2D direction_diff = new_direction - vel.direction;
      double direction_change = direction_diff.length();

      if (direction_change > DIRECTION_TOLERANCE) {
        vel.direction = new_direction;
        registry.emit<ComponentBuilder>(
            entity, registry.get_component_key<Velocity>(), vel.to_bytes());
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
              Velocity& vel,
              double /*dt*/) override
  {
    Vector2D speed(0.0, 0.0);
    Vector2D direction(0.0, 0.0);

    if (vel.speed != speed || vel.direction != direction) {
      vel.speed.x = 0.0;
    }
    vel.speed.y = 0.0;
    vel.direction.x = 0.0;
    vel.direction.y = 0.0;
    registry.emit<ComponentBuilder>(
        entity, registry.get_component_key<Velocity>(), vel.to_bytes());
  }
};

class FollowTargetPattern : public MovementPattern
{
public:
  void update(Registry::Entity entity,
              Registry& registry,
              MovementBehavior& /*behavior*/,
              Position& /*pos*/,
              Velocity& vel,
              double /*dt*/) override
  {
    if (!registry.has_component<Follower>(entity)) {
      registry.add_component(entity, Follower());
      vel.direction.x = -1.0;
      vel.direction.y = 0.0;
      registry.emit<ComponentBuilder>(
          entity, registry.get_component_key<Velocity>(), vel.to_bytes());
    }
    if (!registry.has_component<InteractionZone>(entity)) {
      registry.add_component(entity, InteractionZone(1.5));
      vel.direction.x = -1.0;
      vel.direction.y = 0.0;
      registry.emit<ComponentBuilder>(
          entity, registry.get_component_key<Velocity>(), vel.to_bytes());
    }
  }
};
