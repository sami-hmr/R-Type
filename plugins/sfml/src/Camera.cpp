

#include <chrono>
#include <cstdlib>

#include "plugin/components/Camera.hpp"

#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>

#include "SFMLRenderer.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/events/CameraEvents.hpp"

static void move_cam(Position& pos, Camera& cam)
{
  if (cam.moving) {
    if (pos.pos.distanceTo(cam.target) <= cam.speed.length()) {
      pos.pos = cam.target;
    } else {
      pos.pos += (cam.target - pos.pos).normalize() * cam.speed;
    }
  }
}

static void rotate_cam(Camera& cam)
{
  if (cam.rotating) {
    if (std::abs(cam.rotation - cam.next_rotation) <= cam.rotation_speed) {
      cam.rotation = cam.next_rotation;
      cam.rotation_speed = 0;
      cam.rotating = false;
    } else if (cam.rotation < cam.next_rotation) {
      cam.rotation += cam.rotation_speed;
    } else if (cam.rotation > cam.next_rotation) {
      cam.rotation -= cam.rotation_speed;
    }
  }
}

static void zoom_cam(Camera& cam)
{
  if (cam.zooming) {
    if (cam.size.distanceTo(cam.next_size) <= cam.speed.length()) {
      cam.size = cam.next_size;
      cam.zooming = false;
    } else {
      cam.size += (cam.next_size - cam.size).normalize() * cam.speed;
    }
  }
}

static inline double randn()
{
  return -1 + (2 * static_cast<double>(std::rand()) / RAND_MAX);
}

static void shake_cam(
    Camera& cam,
    sf::Vector2f& center,
    double& display_rotation,
    std::chrono::time_point<std::chrono::high_resolution_clock> now)
{
  if (!cam.shaking) {
    return;
  }
  std::chrono::duration<double> duration = now - cam.shake_start_time;
  if (duration.count() >= cam.shake_duration) {
    cam.shaking = false;
    cam.shaking_trauma = 0;
    cam.shaking_angle = 0;
    cam.shaking_offset = 0;
    return;
  }

  double ratio = duration.count() / cam.shake_duration;
  double effective_trauma = cam.shaking_trauma * (1.0 - (ratio * ratio));

  double angle = cam.shaking_angle * effective_trauma * randn();
  sf::Vector2f offset(cam.shaking_offset * effective_trauma * randn(),
                      cam.shaking_offset * effective_trauma * randn());

  display_rotation += angle;
  center += offset;
}

void SFMLRenderer::camera_system(Registry& r)
{
  sf::Vector2u window_size = _window.getSize();

  for (auto&& [pos, cam] : Zipper<Position, Camera>(r)) {
    if (!_camera_initialized) {
      pos.pos = cam.target;
      _camera_initialized = true;
      this->_view.setCenter(sf::Vector2f(
          static_cast<float>((pos.pos.x + 1.0) * window_size.x / 2.0),
          static_cast<float>((pos.pos.y + 1.0) * window_size.y / 2.0)));
      sf::Vector2f size = {static_cast<float>(cam.size.x * window_size.x),
                           static_cast<float>(cam.size.y * window_size.y)};
      this->_view.setRotation(sf::degrees(static_cast<float>(cam.rotation)));
      this->_view.setSize(size);
      this->_window.setView(this->_view);
    }
    move_cam(pos, cam);
    rotate_cam(cam);
    zoom_cam(cam);

    sf::Vector2f new_center(
        static_cast<float>((pos.pos.x + 1.0) * window_size.x / 2.0),
        static_cast<float>((pos.pos.y + 1.0) * window_size.y / 2.0));
    double display_rotation = cam.rotation;
    shake_cam(cam, new_center, display_rotation, r.clock().now());
    this->_view.setCenter(new_center);

    this->_view.setRotation(
        sf::degrees(static_cast<float>(cam.rotation + display_rotation)));
    sf::Vector2f new_size = {static_cast<float>(cam.size.x * window_size.x),
                             static_cast<float>(cam.size.y * window_size.y)};
    this->_view.setSize(new_size);

    this->_window.setView(this->_view);
    break;
  }
}
