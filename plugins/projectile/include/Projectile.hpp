#pragma once

#include <string>
#include <vector>

#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "Json/JsonParser.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Events.hpp"
#include "plugin/events/CollisionEvent.hpp"

class Projectile : public APlugin
{
public:
  Projectile(Registry& r, EntityLoader& l);

private:
  void init_temporal(Registry::Entity entity, JsonObject const& obj);
  void init_fragile(Registry::Entity entity, JsonObject const& obj);
  void init_owner(Registry::Entity entity, JsonObject const& obj);

  void temporal_system(Registry& reg);
  void on_collision(const CollisionEvent& c);
};
