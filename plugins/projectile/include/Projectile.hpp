#pragma once

#include <string>
#include <vector>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Events.hpp"

class Projectile : public APlugin
{
public:
  Projectile(Registery& r, EntityLoader& l);

private:
  void init_temporal(Registery::Entity entity, JsonObject const& obj);
  void init_fragile(Registery::Entity entity, JsonObject const& obj);

  void temporal_system(Registery& reg);
  void on_collision(const CollisionEvent& c);
};
