#pragma once

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Enemy.hpp"
#include "plugin/components/SpawnPoint.hpp"

class Enemy : public APlugin
{
public:
  Enemy(Registry& r, EntityLoader& l);
  ~Enemy() override = default;

private:
  void init_enemy(Registry::Entity const& entity, JsonObject const& obj);
  void init_spawn_point(Registry::Entity const& entity, JsonObject const& obj);

  void spawn_point_system(Registry& r);
};
