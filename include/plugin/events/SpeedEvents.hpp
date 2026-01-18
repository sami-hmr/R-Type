#pragma once

#include "EventMacros.hpp"
#include "ecs/Entity.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct SpeedModifierEvent
{
  Ecs::Entity target;
  Ecs::Entity source;
  double multiplier;

  CHANGE_ENTITY(result.target = map.at(target); result.source = map.at(source);)

  SpeedModifierEvent(Ecs::Entity t, Ecs::Entity s, double m)
      : target(t)
      , source(s)
      , multiplier(m)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      SpeedModifierEvent,
      ([](Ecs::Entity const& t, Ecs::Entity const& s, double m)
       { return SpeedModifierEvent(t, s, m); }),
      parseByte<Ecs::Entity>(),
      parseByte<Ecs::Entity>(),
      parseByte<double>())

  DEFAULT_SERIALIZE(type_to_byte(this->target),
                    type_to_byte(this->source),
                    type_to_byte(this->multiplier))

  SpeedModifierEvent(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : target(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "entity", entity).value()))
      , source(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "source", entity).value()))
      , multiplier(get_value_copy<double>(r, e, "multiplier").value())
  {
  }
};

struct SpeedSwitcherEvent
{
  Ecs::Entity target;
  Ecs::Entity source;
  double new_speed;

  CHANGE_ENTITY(result.target = map.at(target); result.source = map.at(source);)

  SpeedSwitcherEvent(Ecs::Entity t, Ecs::Entity s, double ns)
      : target(t)
      , source(s)
      , new_speed(ns)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      SpeedSwitcherEvent,
      ([](Ecs::Entity const& t, Ecs::Entity const& s, double ns)
       { return SpeedSwitcherEvent(t, s, ns); }),
      parseByte<Ecs::Entity>(),
      parseByte<Ecs::Entity>(),
      parseByte<double>())

  DEFAULT_SERIALIZE(type_to_byte(this->target),
                    type_to_byte(this->source),
                    type_to_byte(this->new_speed))

  SpeedSwitcherEvent(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : target(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "entity", entity).value()))
      , source(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "source", entity).value()))
      , new_speed(get_value_copy<double>(r, e, "new_speed", entity).value())
  {
  }
};
