#pragma once

#include "EventMacros.hpp"

/**
 * @brief Event to rotate the raycasting camera
 * @note angle is in DEGREES (converted to radians internally by rotate())
 */
struct RaycastingCameraRotateEvent {
    double angle;  ///< Rotation delta in degrees

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