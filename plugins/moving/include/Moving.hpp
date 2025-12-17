#pragma once

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "ecs/SparseArray.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"

class Moving : public APlugin
{
public:
  Moving(Registry& r, EntityLoader& l);

private:
  void init_pos(Registry::Entity const& entity, JsonObject& obj);
  void init_velocity(Registry::Entity const& entity, JsonObject& obj);
  void init_facing(Registry::Entity const& entity, JsonObject& obj);

  void moving_system(Registry&);
};
