/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Camera
*/

#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

class Camera
{
public:
  Camera() = default;

  Camera(Vector2D size,
         Vector2D target,
         Vector2D speed,
         Vector2D next_size = Vector2D(1, 1),
         double rotation = 0,
         double next_rotation = 0,
         double rotation_speed = 0,
         double moving_offset = 0,
         double shaking_trauma = 0,
         double shaking_angle = 0,
         double shaking_offset = 0,
         double shake_duration = 0,
         bool moving = false,
         bool zooming = false,
         bool shaking = false,
         bool rotating = false)
      : size(size)
      , target(target)
      , speed(speed)
      , next_size(next_size)
      , rotation(rotation)
      , next_rotation(next_rotation)
      , rotation_speed(rotation_speed)
      , moving_offset(moving_offset)
      , shaking_trauma(shaking_trauma)
      , shaking_angle(shaking_angle)
      , shaking_offset(shaking_offset)
      , shake_duration(shake_duration)
      , moving(moving)
      , zooming(zooming)
      , shaking(shaking)
      , rotating(rotating)
  {
  }

  ~Camera() = default;

  Vector2D size; /**in screen percentage. */
  Vector2D target;
  Vector2D speed; /**also in screen percentage */
  Vector2D next_size = Vector2D(1, 1);
  double rotation = 0;
  double next_rotation = 0;
  double rotation_speed = 0;
  double moving_offset = 0;
  double shaking_trauma = 0;
  double shaking_angle = 0;
  double shaking_offset = 0;
  double shake_duration = 0;
  bool moving = false; /**Moving -> following the target. */
  bool zooming = false;
  bool shaking = false;
  bool rotating = false;
  std::reference_wrapper<Vector2D> target_ref = std::ref(target);
  std::chrono::time_point<std::chrono::high_resolution_clock> shake_start_time;

  DEFAULT_BYTE_CONSTRUCTOR(Camera,
                           (
                               [](Vector2D size,
                                  Vector2D target,
                                  Vector2D speed,
                                  Vector2D next_size,
                                  double rotation,
                                  double next_rotation,
                                  double rotation_speed,
                                  double moving_offset,
                                  double shaking_trauma,
                                  double shaking_angle,
                                  double shaking_offset,
                                  double shake_duration,
                                  bool moving,
                                  bool zooming,
                                  bool shaking,
                                  bool rotating)
                               {
                                 return Camera(size,
                                               target,
                                               speed,
                                               next_size,
                                               rotation,
                                               next_rotation,
                                               rotation_speed,
                                               moving_offset,
                                               shaking_trauma,
                                               shaking_angle,
                                               shaking_offset,
                                               shake_duration,
                                               moving,
                                               zooming,
                                               shaking,
                                               rotating);
                               }),
                           parseVector2D(),
                           parseVector2D(),
                           parseVector2D(),
                           parseVector2D(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<double>(),
                           parseByte<bool>(),
                           parseByte<bool>(),
                           parseByte<bool>(),
                           parseByte<bool>())

  CHANGE_ENTITY_DEFAULT

  DEFAULT_SERIALIZE(vector2DToByte(this->size),
                    vector2DToByte(this->target),
                    vector2DToByte(this->speed),
                    vector2DToByte(this->next_size),
                    type_to_byte(this->rotation),
                    type_to_byte(this->next_rotation),
                    type_to_byte(this->rotation_speed),
                    type_to_byte(this->moving_offset),
                    type_to_byte(this->shaking_trauma),
                    type_to_byte(this->shaking_angle),
                    type_to_byte(this->shaking_offset),
                    type_to_byte(this->shake_duration),
                    type_to_byte(this->moving),
                    type_to_byte(this->zooming),
                    type_to_byte(this->shaking),
                    type_to_byte(this->rotating))

  HOOKABLE(Camera,
           HOOK(size),
           HOOK(target),
           HOOK(speed),
           HOOK(next_size),
           HOOK(rotation),
           HOOK(next_rotation),
           HOOK(rotation_speed),
           HOOK(moving_offset),
           HOOK(shaking_trauma),
           HOOK(shaking_angle),
           HOOK(shaking_offset),
           HOOK(shake_duration),
           HOOK(moving),
           HOOK(zooming),
           HOOK(shaking),
           HOOK(rotating))
};

#endif /* !CAMERA_HPP_ */
