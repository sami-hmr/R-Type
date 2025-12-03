#pragma once

#include <string>
#include <vector>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/events/Events.hpp"
#include "plugin/EntityLoader.hpp"

class Projectile : public APlugin
{
public:
  Projectile(Registery& r, EntityLoader& l);

private:
  void init_temporal(Registery::Entity entity, JsonVariant const& config);
  void init_fragile(Registery::Entity entity, JsonVariant const& config);
  void init_owner(Registery::Entity entity, JsonVariant const& config);

  void temporal_system(Registery& reg);
  void on_collision(const CollisionEvent& c);
};
