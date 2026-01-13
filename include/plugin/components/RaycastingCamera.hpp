#pragma once

#include <cmath>

#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"


struct RaycastingCamera {
    double angle; 
    double fov;
    int nb_rays;

    RaycastingCamera(double angle, double fov, int nb_rays)
        : angle(angle), fov(fov), nb_rays(nb_rays)
    {
    }

    Vector2D get_direction() const
    {
        return {std::cos(angle), std::sin(angle)};
    }

    void rotate(double delta)
    {
        angle = std::fmod(angle +(delta * M_PI / 180.0), 2 * M_PI);
    }

    CHANGE_ENTITY_DEFAULT

    DEFAULT_BYTE_CONSTRUCTOR(RaycastingCamera,
                             ([](double angle, double fov, int nb_rays)
                              { return (RaycastingCamera) {angle, fov, nb_rays}; }),
                             parseByte<double>(),
                             parseByte<double>(),
                             parseByte<int>())

    DEFAULT_SERIALIZE(type_to_byte(this->angle),
                      type_to_byte(this->fov),
                      type_to_byte(this->nb_rays))

    HOOKABLE(RaycastingCamera, HOOK(angle), HOOK(fov), HOOK(nb_rays))
};


struct RaycastingData {
  Vector2D cam_pos;
  Vector2D map_size;
  double cam_angle;
  double fov;
  double angle_step;
  int nb_rays;
};