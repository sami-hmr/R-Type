
#ifndef ANIMATIONEVENTS_HPP_

#  include <string>

#  include "ByteParser/ByteParser.hpp"
#  include "ecs/Registry.hpp"
#  include "plugin/Byte.hpp"
#  include "plugin/Hooks.hpp"
#  include "plugin/events/EventMacros.hpp"

struct AnimationEndEvent
{
  std::string name;
  Registry::Entity entity;

  CHANGE_ENTITY(result.entity = map.at(entity);)

  AnimationEndEvent(std::string name, Registry::Entity entity)
      : name(std::move(name))
      , entity(entity) {};

  AnimationEndEvent(Registry& r, JsonObject const& e)
      : name(get_value_copy<std::string>(r, e, "name").value())
      , entity(get_value_copy<int>(r, e, "entity").value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(AnimationEndEvent,
                           ([](std::string n, Registry::Entity e)
                            { return AnimationEndEvent(std::move(n), e); }),
                           parseByteString(),
                           parseByte<Registry::Entity>())

  DEFAULT_SERIALIZE(string_to_byte(this->name), type_to_byte(this->entity))
};

struct AnimationStartEvent
{
  std::string name;
  Registry::Entity entity;

  CHANGE_ENTITY(result.entity = map.at(entity);)

  AnimationStartEvent(std::string name, Registry::Entity entity)
      : name(std::move(name))
      , entity(entity) {};

  AnimationStartEvent(Registry& r, JsonObject const& e)
      : name(get_value_copy<std::string>(r, e, "name").value())
      , entity(get_value_copy<int>(r, e, "entity").value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(AnimationStartEvent,
                           ([](std::string n, Registry::Entity e)
                            { return AnimationStartEvent(std::move(n), e); }),
                           parseByteString(),
                           parseByte<Registry::Entity>())

  DEFAULT_SERIALIZE(string_to_byte(this->name), type_to_byte(this->entity))
};

struct PlayAnimationEvent
{
  std::string name;
  Registry::Entity entity;
  double framerate;
  bool loop;
  bool rollback;

  CHANGE_ENTITY(result.entity = map.at(entity);)

  PlayAnimationEvent(std::string name,
                     Registry::Entity entity,
                     double framerate,
                     bool loop,
                     bool rollback)
      : name(std::move(name))
      , entity(entity)
      , framerate(framerate)
      , loop(loop)
      , rollback(rollback)
  {
  }

  PlayAnimationEvent(Registry& r, JsonObject const& e)
      : name(get_value_copy<std::string>(r, e, "name").value())
      , entity(get_value_copy<int>(r, e, "entity").value())
      , framerate(get_value_copy<double>(r, e, "framerate").value())
      , loop(get_value_copy<bool>(r, e, "loop").value())
      , rollback(get_value_copy<bool>(r, e, "rollback").value())
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      PlayAnimationEvent,
      ([](std::string n, Registry::Entity e, double fr, bool l, bool rb)
       { return PlayAnimationEvent(std::move(n), e, fr, l, rb); }),
      parseByteString(),
      parseByte<Registry::Entity>(),
      parseByte<double>(),
      parseByte<bool>(),
      parseByte<bool>())

  DEFAULT_SERIALIZE(string_to_byte(this->name),
                    type_to_byte(this->entity),
                    type_to_byte(this->framerate),
                    type_to_byte(this->loop),
                    type_to_byte(this->rollback))
};

#  define ANIMATIONEVENTS_HPP_
#endif /* !ANIMATIONEVENTS_HPP_ */
