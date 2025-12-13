#include "Projectile.hpp"

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Fragile.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/components/Temporal.hpp"
#include "plugin/events/CollisionEvent.hpp"

Projectile::Projectile(Registry& r, EntityLoader& l)
    : APlugin(r,
              l,
              {"moving", "collision"},
              {COMP_INIT(Temporal, Temporal, init_temporal),
               COMP_INIT(Fragile, Fragile, init_fragile)})
{
  this->_registry.get().register_component<Temporal>("projectile:Temporal");
  this->_registry.get().register_component<Fragile>("projectile:Fragile");

  this->_registry.get().add_system<>(
      [this](Registry& r)
      { this->temporal_system(r); },
      2);
  this->_registry.get().add_system<>(
      [this](Registry& r)
      { this->fragile_system(r); });

  this->_registry.get().on<CollisionEvent>("CollisionEvent", [this](const CollisionEvent& event)
                                            { this->on_collision(event); });
}

void Projectile::init_temporal(Registry::Entity entity, JsonObject const& obj)
{
  auto const& lifetime = get_value<Temporal, double>(
      this->_registry.get(), obj, entity, "lifetime");

  if (!lifetime) {
    std::cerr << "Error loading Position component: unexpected value type "
                 "(expected lifetime: double)\n";
    return;
  }

  this->_registry.get().emplace_component<Temporal>(entity, lifetime.value());
}

void Projectile::init_fragile(Registry::Entity entity, JsonObject const& obj)
{
  auto const& hits =
      get_value<Fragile, int>(this->_registry.get(), obj, entity, "hits");

  if (!hits) {
    std::cerr << "Error loading fragile component: unexpected value type or "
                 "missing value in JsonObject\n";
    return;
  }
  this->_registry.get().emplace_component<Fragile>(
      entity, hits.value(), fragile_cooldown);
}

void Projectile::temporal_system(Registry& reg)
{
  double dt = reg.clock().delta_seconds();

  for (auto&& [i, temporal] : ZipperIndex<Temporal>(reg)) {
    if (!reg.is_entity_dying(i)) {
      temporal.elapsed += dt;

      if (temporal.elapsed >= temporal.lifetime) {
        reg.kill_entity(i);
      }
    }
  }
}

void Projectile::fragile_system(Registry& reg)
{
  double dt = reg.clock().delta_seconds();

  for (auto&& [i, fragile] : ZipperIndex<Fragile>(reg)) {
    if (!reg.is_entity_dying(i)) {
      fragile.fragile_delta += dt;
    }
  }
}

void Projectile::on_collision(const CollisionEvent& event)
{
  auto& fragiles = this->_registry.get().get_components<Fragile>();

  if (!this->_registry.get().has_component<Fragile>(event.a)) {
    return;
  }

  if (this->_registry.get().has_component<Team>(event.a)
      && this->_registry.get().has_component<Team>(event.b))
  {
    auto& teams = this->_registry.get().get_components<Team>();

    if (teams[event.a]->name == teams[event.b]->name) {
      return;
    }
  }

  if (fragiles[event.a]->fragile_delta >= fragile_cooldown) {
    fragiles[event.a]->fragile_delta = 0.0;
    if (fragiles[event.a]->counter >= fragiles[event.a]->hits
        && !this->_registry.get().is_entity_dying(event.a))
    {
      this->_registry.get().kill_entity(event.a);
      return;
    }
    fragiles[event.a]->counter += 1;
  }
}

extern "C"
{
void* entry_point(Registry& r, EntityLoader& e)
{
  return new Projectile(r, e);
}
}
