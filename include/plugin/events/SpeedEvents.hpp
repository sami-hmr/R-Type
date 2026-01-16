#pragma once

#include "EventMacros.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct SpeedModifierEvent
{
  Registry::Entity target;
  Registry::Entity source;
  double multiplier;

  CHANGE_ENTITY(result.target = map.at(target); result.source = map.at(source);)

  SpeedModifierEvent(Registry::Entity t, Registry::Entity s, double m)
      : target(t)
      , source(s)
      , multiplier(m)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(SpeedModifierEvent,
                           ([](Registry::Entity const& t,
                               Registry::Entity const& s,
                               double m) { return SpeedModifierEvent(t, s, m); }),
                           parseByte<Registry::Entity>(),
                           parseByte<Registry::Entity>(),
                           parseByte<double>())

  DEFAULT_SERIALIZE(type_to_byte(this->target),
                    type_to_byte(this->source),
                    type_to_byte(this->multiplier))

  SpeedModifierEvent(Registry& r, JsonObject const& e)
      : target(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "entity").value()))
      , source(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "source").value()))
      , multiplier(get_value_copy<double>(r, e, "multiplier").value())
  {
  }
};

struct SpeedSwitcherEvent
{
  Registry::Entity target;
  Registry::Entity source;
  double new_speed;

  CHANGE_ENTITY(result.target = map.at(target); result.source = map.at(source);)

  SpeedSwitcherEvent(Registry::Entity t, Registry::Entity s, double ns)
      : target(t)
      , source(s)
      , new_speed(ns)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(SpeedSwitcherEvent,
                           ([](Registry::Entity const& t,
                               Registry::Entity const& s,
                               double ns) { return SpeedSwitcherEvent(t, s, ns); }),
                           parseByte<Registry::Entity>(),
                           parseByte<Registry::Entity>(),
                           parseByte<double>())

  DEFAULT_SERIALIZE(type_to_byte(this->target),
                    type_to_byte(this->source),
                    type_to_byte(this->new_speed))

  SpeedSwitcherEvent(Registry& r, JsonObject const& e)
      : target(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "entity").value()))
      , source(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "source").value()))
      , new_speed(get_value_copy<double>(r, e, "new_speed").value())
  {
  }
};
