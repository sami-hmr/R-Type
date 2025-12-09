#include "Projectile.hpp"

#include "Json/JsonParser.hpp"
#include "Logger.hpp"
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Fragile.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/components/Temporal.hpp"
#include "plugin/events/Events.hpp"

Projectile::Projectile(Registery& r, EntityLoader& l)
    : APlugin(r,
              l,
              {"moving", "collision"},
              {COMP_INIT(Temporal, Temporal, init_temporal),
               COMP_INIT(Fragile, Fragile, init_fragile)})
{
  this->_registery.get().register_component<Temporal>();
  this->_registery.get().register_component<Fragile>();

  this->_registery.get().add_system<Temporal>(
      [this](Registery& r, const SparseArray<Temporal>&)
      { this->temporal_system(r); },
      2);
  this->_registery.get().add_system<>(
      [this](Registery& r)
      { this->update_cooldown(r); });

  this->_registery.get().on<CollisionEvent>([this](const CollisionEvent& event)
                                            { this->on_collision(event); });
}

void Projectile::init_temporal(Registery::Entity entity, JsonObject const& obj)
{
  auto const& lifetime = get_value<Temporal, double>(
      this->_registery.get(), obj, entity, "lifetime");

  if (!lifetime) {
    std::cerr << "Error loading Position component: unexpected value type "
                 "(expected lifetime: double)\n";
    return;
  }

  this->_registery.get().emplace_component<Temporal>(entity, lifetime.value());
}

void Projectile::init_fragile(Registery::Entity entity, JsonObject const& obj)
{
  auto const& hits =
      get_value<Fragile, int>(this->_registery.get(), obj, entity, "hits");

  if (!hits) {
    std::cerr << "Error loading fragile component: unexpected value type or "
                 "missing value in JsonObject\n";
    return;
  }
  this->_registery.get().emplace_component<Fragile>(
      entity, hits.value(), fragile_cooldown);
}

void Projectile::temporal_system(Registery& reg)
{
  auto& temporals = reg.get_components<Temporal>();
  double dt = reg.clock().delta_seconds();

  for (auto&& [i, temporal] : ZipperIndex(temporals)) {
    if (!reg.is_entity_dying(i)) {
      temporals[i]->elapsed += dt;

      if (temporal.elapsed >= temporal.lifetime) {
        reg.kill_entity(i);
      }
    }
  }
}

void Projectile::on_collision(const CollisionEvent& event)
{
  auto& fragiles = this->_registery.get().get_components<Fragile>();

  if (!this->_registery.get().has_component<Fragile>(event.a)) {
    return;
  }

  if (this->_registery.get().has_component<Team>(event.a)
      && this->_registery.get().has_component<Team>(event.b))
  {
    auto& teams = this->_registery.get().get_components<Team>();

    if (teams[event.a]->name == teams[event.b]->name) {
      return;
    }
  }

  if (fragiles[event.a]->fragile_delta >= fragile_cooldown) {
    fragiles[event.a]->fragile_delta = 0.0;
    if (fragiles[event.a]->counter >= fragiles[event.a]->hits
        && !this->_registery.get().is_entity_dying(event.a))
    {
      this->_registery.get().kill_entity(event.a);
      return;
    }
    fragiles[event.a]->counter += 1;
  }
}

void Projectile::update_cooldown(Registery& reg)
{
  double dt = reg.clock().delta_seconds();
  auto& fragiles = reg.get_components<Fragile>();

  for (auto&& [i, fragile] : ZipperIndex(fragiles)) {
    if (!reg.is_entity_dying(i)) {
      fragile.fragile_delta += dt;
    }
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new Projectile(r, e);
}
}
