#include "Enemy.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Position.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include <iostream>

Enemy::Enemy(Registry& r, EntityLoader& l)
    : APlugin(
          r,
          l,
          {},
          {COMP_INIT(Enemy, Enemy, init_enemy),
           COMP_INIT(SpawnPoint, SpawnPoint, init_spawn_point)})
{
  _registry.get().register_component<::Enemy>("enemy:Enemy");
  _registry.get().register_component<SpawnPoint>("enemy:SpawnPoint");

  _registry.get().add_system([this](Registry& r) { spawn_point_system(r); });
}

void Enemy::init_enemy(Registry::Entity const& entity, JsonObject const& obj)
{
    return;
}

void Enemy::init_spawn_point(Registry::Entity const& entity, JsonObject const& obj)
{
    return;
}

void Enemy::spawn_point_system(Registry& r)
{
    return;
}

extern "C"
{
  void* entry_point(Registry& r, EntityLoader& e)
  {
    return new Enemy(r, e);
  }
}
