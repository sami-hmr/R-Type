#ifndef CAMERAEVENTS_HPP_
#define CAMERAEVENTS_HPP_

#include "EventMacros.hpp"
#include "ecs/Registry.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct CamAggroEvent
{
  Registry::Entity target;

  CHANGE_ENTITY(result.target = map.at(target))

  CamAggroEvent(Registry::Entity t)
      : target(t)
  {
  }

  CamAggroEvent(Registry& r, JsonObject const& e)
      : target(static_cast<Registry::Entity>(
            get_value_copy<double>(r, e, "target").value()))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(CamAggroEvent,
                           ([](Registry::Entity e)
                            { return CamAggroEvent(e); }),
                           parseByte<Registry::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(this->target))
};

struct CamZoomEvent {
    Vector2D next_size;

    CamZoomEvent(Vector2D size)
        : next_size(size) {}
    CamZoomEvent(Registry& r, JsonObject const& e)
        : next_size(get_value_copy<Vector2D>(r, e, "size").value()) {}

    CHANGE_ENTITY_DEFAULT

    DEFAULT_BYTE_CONSTRUCTOR(CamZoomEvent,
                             ([](Vector2D size)
                              { return CamZoomEvent(size); }),
                             parseVector2D())
    DEFAULT_SERIALIZE(vector2DToByte(this->next_size))
};

struct CamRotateEvent {
    double next_rotation;
    double speed;

    CamRotateEvent(double rotation, double speed)
        : next_rotation(rotation), speed(speed) {}
    CamRotateEvent(Registry& r, JsonObject const& e)
        : next_rotation(get_value_copy<double>(r, e, "rotation").value()), speed(get_value_copy<double>(r, e, "speed").value()) {}

    CHANGE_ENTITY_DEFAULT

    DEFAULT_BYTE_CONSTRUCTOR(CamRotateEvent,
                             ([](double rotation, double speed)
                              { return CamRotateEvent(rotation, speed); }),
                             parseByte<double>(), parseByte<double>())
    DEFAULT_SERIALIZE(type_to_byte(this->next_rotation), type_to_byte(this->speed))
};

struct CamSpeedEvent {
    Vector2D speed;

    CamSpeedEvent(Vector2D speed)
        : speed(speed) {}
    CamSpeedEvent(Registry& r, JsonObject const& e)
        : speed(get_value_copy<Vector2D>(r, e, "speed").value()) {}

    CHANGE_ENTITY_DEFAULT

    DEFAULT_BYTE_CONSTRUCTOR(CamSpeedEvent,
                             ([](Vector2D speed)
                              { return CamSpeedEvent(speed); }),
                             parseVector2D())
    DEFAULT_SERIALIZE(vector2DToByte(this->speed))
};

#endif /* !CAMERAEVENTS_HPP_ */
