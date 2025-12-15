#pragma once

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Spawner.hpp"

class Mob : public APlugin
{
public:
  Mob(Registry& r, EntityLoader& l);
  ~Mob() override = default;

private:
  void init_enemy(Registry::Entity const& entity, JsonObject const& obj);
  void init_spawner(Registry::Entity const& entity, JsonObject const& obj);

  void spawner_system(Registry& r);
};
