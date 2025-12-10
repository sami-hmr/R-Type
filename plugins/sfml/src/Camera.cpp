

#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>

#include "SFMLRenderer.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/events/CameraEvents.hpp"

void SFMLRenderer::init_cam(Registry::Entity const &entity,
                            JsonObject const& obj)
{
  Vector2D size(0.5, 0.5);
  Vector2D target(0.0, 0.0);
  Vector2D speed(0.1, 0.1);

  auto sizeopt = get_value<Camera, Vector2D>(this->_registry.get(), obj, entity, "size", "width", "height");
  if (sizeopt.has_value()) {
    size = sizeopt.value();
  } else {
    std::cerr
        << "Camera component missing size field, using default (50%, 50%)\n";
    return;
  }
  auto targetopt = get_value<Camera, Vector2D>(this->_registry.get(), obj, entity, "target");
  if (targetopt.has_value()) {
    target = targetopt.value();
  } else {
    std::cerr
        << "Camera component missing target field, using default (0, 0)\n";
    return;
  }
  auto speedopt = get_value<Camera, Vector2D>(this->_registry.get(), obj, entity, "speed", "x", "y");
  if (speedopt.has_value()) {
    speed = speedopt.value();
  } else {
    std::cerr
        << "Camera component missing speed field, using default (10%, 15%)\n";
    return;
  }
  _registry.get().emplace_component<Camera>(entity, size, target, speed);
  _registry.get().on<CamAggroEvent>("CamAggroEvent", [this](const CamAggroEvent& e) {
    this->cam_target_event(e);
  });
  _registry.get().on<CamZoomEvent>("CamZoomEvent", [this](const CamZoomEvent& e) {
      this->cam_zoom_event(e);
  });
  _registry.get().on<CamRotateEvent>("CamRotateEvent", [this](const CamRotateEvent& e) {
      this->cam_rotate_event(e);
  });
  _registry.get().on<CamSpeedEvent>("CamSpeedEvent", [this](const CamSpeedEvent& e) {
      this->cam_speed_event(e);
  });
}

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
