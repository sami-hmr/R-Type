#ifndef CAMERAEVENTS_HPP_
#define CAMERAEVENTS_HPP_

#include "EventMacros.hpp"
#include "ecs/Registry.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct CamAggroEvent
{
  Ecs::Entity target;

  CHANGE_ENTITY(result.target = map.at(target))

  CamAggroEvent(Ecs::Entity t)
      : target(t)
  {
  }

  CamAggroEvent(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : target(static_cast<Ecs::Entity>(
            get_value_copy<double>(r, e, "entity", entity).value()))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(CamAggroEvent,
                           ([](Ecs::Entity e)
                            { return CamAggroEvent(e); }),
                           parseByte<Ecs::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(this->target))
};

struct CamMoveEvent
{
  Vector2D target;

  CamMoveEvent(Vector2D target)
      : target(target)
  {
  }

  CamMoveEvent(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : target(get_value_copy<Vector2D>(r, e, "target", entity).value())
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(CamMoveEvent,
                           ([](Vector2D target)
                            { return CamMoveEvent(target); }),
                           parseVector2D())
  DEFAULT_SERIALIZE(vector2DToByte(this->target))
};

struct CamZoomEvent
{
  Vector2D next_size;

  CamZoomEvent(Vector2D size)
      : next_size(size)
  {
  }

  CamZoomEvent(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : next_size(get_value_copy<Vector2D>(r, e, "size", entity).value())
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(CamZoomEvent,
                           ([](Vector2D size) { return CamZoomEvent(size); }),
                           parseVector2D())
  DEFAULT_SERIALIZE(vector2DToByte(this->next_size))
};

struct CamRotateEvent
{
  double next_rotation;
  double speed;

  CamRotateEvent(double rotation, double speed)
      : next_rotation(rotation)
      , speed(speed)
  {
  }

  CamRotateEvent(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : next_rotation(get_value_copy<double>(r, e, "rotation", entity).value())
      , speed(get_value_copy<double>(r, e, "speed", entity).value())
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(CamRotateEvent,
                           ([](double rotation, double speed)
                            { return CamRotateEvent(rotation, speed); }),
                           parseByte<double>(),
                           parseByte<double>())
  DEFAULT_SERIALIZE(type_to_byte(this->next_rotation),
                    type_to_byte(this->speed))
};

struct CamSpeedEvent
{
  Vector2D speed;

  CamSpeedEvent(Vector2D speed)
      : speed(speed)
  {
  }

  CamSpeedEvent(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : speed(get_value_copy<Vector2D>(r, e, "speed", entity).value())
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(CamSpeedEvent,
                           ([](Vector2D speed)
                            { return CamSpeedEvent(speed); }),
                           parseVector2D())
  DEFAULT_SERIALIZE(vector2DToByte(this->speed))
};

#endif /* !CAMERAEVENTS_HPP_ */
