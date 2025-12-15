#include <iostream>

#include "Mob.hpp"

#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Enemy.hpp"
#include "plugin/components/Position.hpp"

Mob::Mob(Registry& r, EntityLoader& l)
    : APlugin(r,
              l,
              {},
              {COMP_INIT(Enemy, Enemy, init_enemy),
               COMP_INIT(Spawner, Spawner, init_spawner)})
{
  _registry.get().register_component<Enemy>("mob:Enemy");
  _registry.get().register_component<Spawner>("mob:Spawner");

  _registry.get().add_system([this](Registry& r) { spawner_system(r); });
}

void Mob::init_enemy(Registry::Entity const& entity, JsonObject const& obj)
{
  return;
}

void Mob::init_spawner(Registry::Entity const& entity, JsonObject const& obj)
{
  return;
}

void Mob::spawner_system(Registry& r)
{
  return;
}

extern "C"
{
void* entry_point(Registry& r, EntityLoader& e)
{
  return new Mob(r, e);
}
}
