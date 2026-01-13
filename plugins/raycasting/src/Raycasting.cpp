#include <iostream>
#include <variant>

#include "Raycasting.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/BasicMap.hpp"
#include "plugin/components/RaycastingCamera.hpp"
#include "plugin/events/CollisionEvent.hpp"
#include "plugin/events/RaycastingCameraEvents.hpp"

/**update the direction depending on the angle in degrees, because in the config
 * file its always 1 -1 */
bool Raycasting::on_update_direction(Registry& r, const UpdateDirection& event)
{
  if (this->changed_direction) {
    this->changed_direction = false;
    return false;
  }
  this->changed_direction = true;
  auto& raycasting_cameras = r.get_components<RaycastingCamera>();
  if (event.entity >= raycasting_cameras.size()) {
    return false;
  }
  auto& camera = raycasting_cameras[event.entity];
  if (!camera.has_value()) {
    return false;
  }

  // Get camera direction and perpendicular (for strafe)
  Vector2D dir = camera->get_direction();
  Vector2D perp = {-dir.y, dir.x};

  // event.x_axis = strafe (left/right), event.y_axis = forward/backward
  // In the original config: y=-1 means forward, y=1 means backward
  // x=-1 means left, x=1 means right
  double forward = -event.y_axis;  // Invert because y=-1 is forward
  double strafe = event.x_axis;

  // Compute the new direction relative to camera angle
  Vector2D new_dir = dir * forward + perp * strafe;

  UpdateDirection new_event = event;
  // Update the event with the rotated direction
  new_event.x_axis = new_dir.x;
  new_event.y_axis = new_dir.y;

  this->_event_manager.get().emit<UpdateDirection>(new_event);
  return true;
}

Raycasting::Raycasting(Registry& r,
                       EventManager& em,
                       EntityLoader& l,
                       std::optional<JsonObject> const& config)
    : APlugin("raycasting",
              r,
              em,
              l,
              {},
              {COMP_INIT(BasicMap, BasicMap, init_basic_map),
               COMP_INIT(Camera, RaycastingCamera, init_cam)})
{
  REGISTER_COMPONENT(BasicMap)
  REGISTER_COMPONENT(RaycastingCamera)

  SUBSCRIBE_EVENT(UpdateDirection,
                  { on_update_direction(this->_registry.get(), event); })

  SUBSCRIBE_EVENT(RaycastingCameraRotateEvent, {
    auto& cameras = this->_registry.get().get_components<RaycastingCamera>();
    for (auto& camera : cameras) {
      if (camera.has_value()) {
        camera->rotate(event.angle);
      }
    }
  })
}

extern "C"
{
void* entry_point(Registry& r,
                  EventManager& em,
                  EntityLoader& e,
                  std::optional<JsonObject> const& config)
{
  return new Raycasting(r, em, e, config);
}
}
