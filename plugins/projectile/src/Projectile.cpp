#include "Projectile.hpp"

#include "Json/JsonParser.hpp"
#include "Logger.hpp"
#include "ecs/Registry.hpp"
#include "ecs/SparseArray.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Fragile.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/components/Temporal.hpp"
#include "plugin/events/Events.hpp"
#include "ecs/zipper/ZipperIndex.hpp"

Projectile::Projectile(Registry& r, EntityLoader& l)
    : APlugin(r,
              l,
              {"moving", "collision"},
              {COMP_INIT(Temporal, Temporal, init_temporal),
               COMP_INIT(Fragile, Fragile, init_fragile)})
{
  this->_registry.get().register_component<Temporal>();
  this->_registry.get().register_component<Fragile>();

  this->_registry.get().add_system<Temporal>(
      [this](Registry& r, const SparseArray<Temporal>&)
      { this->temporal_system(r); },
      2);

  this->_registry.get().on<CollisionEvent>([this](const CollisionEvent& event)
                                            { this->on_collision(event); });
}

void Projectile::init_temporal(Registry::Entity entity,
                               JsonObject const& obj)
{
  auto const& lifetime = get_value<double>(this->_registry.get(), obj, "lifetime");

  if (!lifetime) {
    std::cerr << "Error loading Position component: unexpected value type "
                 "(expected lifetime: double)\n";
    return;
  }

  this->_registry.get().emplace_component<Temporal>(
      entity, lifetime.value());
}

void Projectile::init_fragile(Registry::Entity entity,
                              JsonObject const& /*obj*/)
{
  this->_registry.get().emplace_component<Fragile>(entity);
}

void Projectile::temporal_system(Registry& reg)
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
  if (!this->_registry.get().has_component<Fragile>(event.a)) {
    return;
  }

  if (this->_registry.get().has_component<Team>(event.a) && this->_registry.get().has_component<Team>(event.b)) {
    auto& teams = this->_registry.get().get_components<Team>();

    if (teams[event.a]->name == teams[event.b]->name) {
      return;
    }
  }

  if (!this->_registry.get().is_entity_dying(event.a)) {
    this->_registry.get().kill_entity(event.a);
  }
}

extern "C"
{
void* entry_point(Registry& r, EntityLoader& e)
{
  return new Projectile(r, e);
}
}
