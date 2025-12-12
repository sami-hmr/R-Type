
#include "plugin/events/CameraEvents.hpp"
#include "UI.hpp"
#include "plugin/components/Camera.hpp"
#include "plugin/components/Position.hpp"


void UI::cam_target_event(const CamAggroEvent& e)
{
  Vector2D target = {0, 0};
  SparseArray<Position> positions = _registry.get().get_components<Position>();

  if (!this->_registry.get().has_component<Position>(e.target)) {
    return;
  }
  target = positions.at(e.target).value().pos;
  for (auto&& [pos, cam] : Zipper(_registry.get().get_components<Position>(),
                                  _registry.get().get_components<Camera>()))
  {
    cam.target = target;
    cam.moving = true;
  }
}

void UI::cam_zoom_event(const CamZoomEvent &e)
{
    for (auto&& [cam] : Zipper(_registry.get().get_components<Camera>()))
    {
        cam.next_size = e.next_size;
        cam.zooming = true;
    }
}

void UI::cam_speed_event(const CamSpeedEvent &e)
{
    for (auto&& [cam] : Zipper(_registry.get().get_components<Camera>()))
    {
        cam.speed = e.speed;
    }
}

void UI::cam_rotate_event(const CamRotateEvent &e)
{
    for (auto&& [cam] : Zipper(_registry.get().get_components<Camera>()))
    {
        cam.next_rotation = e.next_rotation;
        cam.rotation_speed = e.speed;
        cam.rotating = true;
    }
}