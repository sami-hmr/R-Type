#include <cmath>

#include "Raycasting.hpp"
#include "ecs/InitComponent.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/components/RaycastingCamera.hpp"

void Raycasting::init_cam(Ecs::Entity& e, const JsonObject& obj)
{
  double angle = 0.0;

  if (obj.contains("angle")) {
    angle = get_value<RaycastingCamera, double>(
                this->_registry.get(), obj, e, "angle")
                .value();
    angle = angle * std::numbers::pi / 180.0;
  } else if (obj.contains("target")) {
    Vector2D target = get_value<RaycastingCamera, Vector2D>(
                          this->_registry.get(), obj, e, "target", "x", "y")
                          .value();
    angle = std::atan2(target.y, target.x);
  } else {
    std::cerr << "[Raycasting] Camera component missing 'angle' or 'target' field"
              << std::endl;
    return;
  }

  double fov = 60.0;
  if (obj.contains("fov")) {
    fov = get_value<RaycastingCamera, double>(
              this->_registry.get(), obj, e, "fov")
              .value();
  }

  int nb_rays = 320;
  if (obj.contains("nb_rays")) {
    nb_rays = get_value<RaycastingCamera, int>(
                  this->_registry.get(), obj, e, "nb_rays")
                  .value();
  }

  init_component<RaycastingCamera>(this->_registry.get(),
                                   this->_event_manager.get(),
                                   e,
                                   angle,
                                   fov,
                                   nb_rays);
}
