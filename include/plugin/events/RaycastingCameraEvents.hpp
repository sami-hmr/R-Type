#pragma once

#include "EventMacros.hpp"

struct RaycastingCameraRotateEvent {
    double angle;

    RaycastingCameraRotateEvent(double angle)
        : angle(angle)
    {
    }

    RaycastingCameraRotateEvent(Registry& r, JsonObject const& e)
        : angle(get_value_copy<double>(r, e, "angle").value())
    {
    }

    CHANGE_ENTITY_DEFAULT

    DEFAULT_BYTE_CONSTRUCTOR(RaycastingCameraRotateEvent,
                             ([](double angle)
                              { return RaycastingCameraRotateEvent(angle); }),
                             parseByte<double>())

    DEFAULT_SERIALIZE(type_to_byte(this->angle))
};