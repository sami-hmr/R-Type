

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

static void rotate_cam(Camera &cam)
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

static void zoom_cam(Camera &cam) {
  if (cam.zooming) {
    if (cam.size.distanceTo(cam.next_size) <= cam.speed.length()) {
      cam.size = cam.next_size;
      cam.zooming = false;
    } else {
      cam.size += (cam.next_size - cam.size).normalize() * cam.speed;
    }
  }
}

static void shake_cam(Camera &cam) {
  if (cam.shaking) {
    // Vector2D offset();
  }
}

void SFMLRenderer::camera_system(Registry& r,
                                 SparseArray<Position>& positions,
                                 SparseArray<Camera>& cameras)
{
  sf::Vector2u window_size = _window.getSize();

  for (auto&& [pos, cam] : Zipper(positions, cameras)) {
    move_cam(pos, cam);
    rotate_cam(cam);
    zoom_cam(cam);
    sf::Vector2f new_center(
        static_cast<float>((pos.pos.x + 1.0) * window_size.x / 2.0),
        static_cast<float>((pos.pos.y + 1.0) * window_size.y / 2.0));
    this->_view.setCenter(new_center);

    this->_view.setRotation(sf::degrees(static_cast<float>(cam.rotation)));
    sf::Vector2f new_size = {static_cast<float>(cam.size.x * window_size.x),
                             static_cast<float>(cam.size.y * window_size.y)};
    this->_view.setSize(new_size);

    this->_window.setView(this->_view);
    break;
  }
}
