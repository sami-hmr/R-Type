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
