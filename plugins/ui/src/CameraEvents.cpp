
#include "plugin/events/CameraEvents.hpp"

#include "UI.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/components/Camera.hpp"
#include "plugin/components/Position.hpp"
#include "ecs/InitComponent.hpp"

void UI::cam_target_event(const CamAggroEvent& e)
{
  Vector2D target = {0, 0};
  SparseArray<Position> positions = _registry.get().get_components<Position>();

  if (!this->_registry.get().has_component<Position>(e.target)) {
    return;
  }
  target = positions.at(e.target).value().pos;
  for (auto&& [pos, cam] : Zipper<Position, Camera>(this->_registry.get())) {
    cam.target = target;
    cam.moving = true;
  }
}

void UI::cam_move_event(const CamMoveEvent& e)
{
  for (auto&& [cam] : Zipper<Camera>(this->_registry.get())) {
    cam.target = e.target;
    cam.moving = true;
  }
}

void UI::cam_zoom_event(const CamZoomEvent& e)
{
  for (auto&& [cam] : Zipper<Camera>(this->_registry.get())) {
    cam.next_size = e.next_size;
    cam.zooming = true;
  }
}

void UI::cam_speed_event(const CamSpeedEvent& e)
{
  for (auto&& [cam] : Zipper<Camera>(this->_registry.get())) {
    cam.speed = e.speed;
  }
}

void UI::cam_rotate_event(const CamRotateEvent& e)
{
  for (auto&& [cam] : Zipper<Camera>(this->_registry.get())) {
    cam.next_rotation = e.next_rotation;
    cam.rotation_speed = e.speed;
    cam.rotating = true;
  }
}

void UI::cam_shake_event(const CameraShakeEvent& e)
{
  for (auto&& [cam] : Zipper<Camera>(this->_registry.get())) {
    cam.shaking_trauma = e.trauma;
    cam.shaking_angle = e.angle;
    cam.shaking_offset = e.offset;
    cam.shake_duration = e.duration;
    cam.shake_start_time = this->_registry.get().clock().now();
    cam.shaking = true;
  }
}

void UI::init_cam(Registry::Entity const& entity, JsonObject const& obj)
{
  Vector2D size(0.5, 0.5);
  Vector2D target(0.0, 0.0);
  Vector2D speed(0.1, 0.1);

  auto sizeopt = get_value<Camera, Vector2D>(
      this->_registry.get(), obj, entity, "size", "width", "height");
  if (sizeopt.has_value()) {
    size = sizeopt.value();
  } else {
    std::cerr
        << "Camera component missing size field, using default (50%, 50%)\n";
    return;
  }
  auto targetopt =
      get_value<Camera, Vector2D>(this->_registry.get(), obj, entity, "target");
  if (targetopt.has_value()) {
    target = targetopt.value();
  } else {
    std::cerr
        << "Camera component missing target field, using default (0, 0)\n";
    return;
  }
  auto speedopt = get_value<Camera, Vector2D>(
      this->_registry.get(), obj, entity, "speed", "x", "y");
  if (speedopt.has_value()) {
    speed = speedopt.value();
  } else {
    std::cerr
        << "Camera component missing speed field, using default (10%, 15%)\n";
    return;
  }
  init_component(this->_registry.get(),
                 this->_event_manager.get(),
                 entity,
                 Camera(size, target, speed));
}
