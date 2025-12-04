#include "Projectile.hpp"

#include "Json/JsonParser.hpp"
#include "Logger.hpp"
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Fragile.hpp"
#include "plugin/components/Owner.hpp"
#include "plugin/components/Temporal.hpp"
#include "plugin/events/Events.hpp"

Projectile::Projectile(Registery& r, EntityLoader& l)
    : APlugin(r,
              l,
              {"moving", "collision"},
              {COMP_INIT(Temporal, Temporal, init_temporal),
               COMP_INIT(Fragile, Fragile, init_fragile),
               COMP_INIT(Owner, Owner, init_owner)})
{
  this->_registery.get().register_component<Temporal>();
  this->_registery.get().register_component<Fragile>();
  this->_registery.get().register_component<Owner>();

  this->_registery.get().add_system<Temporal>(
      [this](Registery& r, const SparseArray<Temporal>&)
      { this->temporal_system(r); },
      2);

  this->_registery.get().on<CollisionEvent>([this](const CollisionEvent& event)
                                            { this->on_collision(event); });
}

void Projectile::init_temporal(Registery::Entity entity,
                               JsonObject const& obj)
{
  auto const& lifetime = get_value<double>(this->_registery.get(), obj, "lifetime");

  if (!lifetime) {
    std::cerr << "Error loading Position component: unexpected value type "
                 "(expected lifetime: double)\n";
    return;
  }

  this->_registery.get().emplace_component<Temporal>(
      entity, lifetime.value());
}

void Projectile::init_fragile(Registery::Entity entity,
                              JsonObject const& /*obj*/)
{
  this->_registery.get().emplace_component<Fragile>(entity);
}

void Projectile::init_owner(Registery::Entity entity,
                              JsonObject const& obj)
{
  auto const& owner = get_value<int>(this->_registery.get(), obj, "owner");

  if (!owner || owner < 0) {
    std::cerr << "Error loading Position component: unexpected value type "
                 "(expected owner: unsigned int)\n";
    return;
  }

  this->_registery.get().emplace_component<Owner>(
      entity, owner.value());
}

void Projectile::temporal_system(Registery& reg)
{
  auto& temporals = reg.get_components<Temporal>();
  double dt = reg.clock().delta_seconds();

  for (size_t i = 0; i < temporals.size(); ++i) {
    if (temporals[i].has_value() && !reg.is_entity_dying(i)) {
      temporals[i]->elapsed += dt;

      if (temporals[i]->elapsed >= temporals[i]->lifetime) {
        reg.kill_entity(i);
      }
    }
  }
}

void Projectile::on_collision(const CollisionEvent& event)
{
  if (!this->_registery.get().has_component<Fragile>(event.a)) {
    return;
  }

  if (this->_registery.get().has_component<Owner>(event.a)) {
    auto& owners = this->_registery.get().get_components<Owner>();

    if (owners[event.a]->entity_id == event.b) {
      return;
    }
  }

  if (!this->_registery.get().is_entity_dying(event.a)) {
    this->_registery.get().kill_entity(event.a);
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new Projectile(r, e);
}
}
