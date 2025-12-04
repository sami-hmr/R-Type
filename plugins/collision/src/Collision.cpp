#include <iostream>
#include <iterator>
#include <stdexcept>
#include <variant>
#include <vector>

#include "Collision.hpp"

#include "Json/JsonParser.hpp"
#include "Logger.hpp"
#include "algorithm/QuadTreeCollision.hpp"
#include "ecs/Registery.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/Byte.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Collidable.hpp"
#include "plugin/components/Damage.hpp"
#include "plugin/components/Health.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/components/Velocity.hpp"
#include "plugin/events/Events.hpp"

Collision::Collision(Registery& r, EntityLoader& l)
    : APlugin(r, l, {"moving"}, {COMP_INIT(Collidable, Collidable, init_collision)})
{
  _registery.get().register_component<Collidable>("collision:Collidable");
  _registery.get().register_component<Team>("collision:Team");

  _collision_algo = std::make_unique<QuadTreeCollision>(2.0, 2.0);

  if (!_collision_algo) {
    LOGGER("COLLISION", LogLevel::ERROR, "Error loading collision algorithm")
  }

  _registery.get().add_system<Position, Collidable>(
      [this](Registery& r,
             const SparseArray<Position>& pos,
             const SparseArray<Collidable>& col)
      { this->collision_system(r, pos, col); },
      3);

  this->_registery.get().on<CollisionEvent>([this](const CollisionEvent& c)
                                            { this->on_collision(c); });
}

void Collision::set_algorithm(std::unique_ptr<ICollisionAlgorithm> algo)
{
  _collision_algo = std::move(algo);
}

void Collision::init_collision(Registery::Entity const& entity,
                               JsonObject const& obj)
{
  auto const& width = get_value<double>(this->_registery.get(), obj, "width");
  auto const& height = get_value<double>(this->_registery.get(), obj, "height");
  auto const& type_str =
      get_value<std::string>(this->_registery.get(), obj, "collision_type");

  if (!width || !height || !type_str) {
    std::cerr
        << "Error loading collision component: unexpected value type (expected "
           "width: double and height: double) or missing value in JsonObject\n";
    return;
  }

  CollisionType type = CollisionType::Solid;
  if (type_str.value() == "Trigger" || type_str.value() == "trigger") {
    type = CollisionType::Trigger;
  } else if (type_str == "Solid" || type_str == "solid") {
    type = CollisionType::Solid;
  }

  this->_registery.get().emplace_component<Collidable>(
      entity, width.value(), height.value(), type, true);
}

void Collision::collision_system(Registery& reg,
                                 const SparseArray<Position>& positions,
                                 const SparseArray<Collidable>& collidables)
{
  if (!_collision_algo) {
    return;
  }

  std::vector<ICollisionAlgorithm::CollisionEntity> entities;

  std::size_t max_size = std::min(positions.size(), collidables.size());
  for (std::size_t i = 0; i < max_size; ++i) {
    if (reg.has_component<Position>(i) && reg.has_component<Collidable>(i)
        && collidables[i]->is_active)
    {
      entities.push_back(ICollisionAlgorithm::CollisionEntity {
          .entity_id = i,
          .bounds = Rect {.x = positions[i]->x,
                          .y = positions[i]->y,
                          .width = collidables[i]->width,
                          .height = collidables[i]->height}});
    }
  }

  _collision_algo->update(entities);
  auto collisions = _collision_algo->detect_collisions(entities);

  for (auto const& collision : collisions) {
    std::size_t entity_a = collision.entity_a;
    std::size_t entity_b = collision.entity_b;

    this->_registery.get().emit<CollisionEvent>(entity_a, entity_b);
    this->_registery.get().emit<CollisionEvent>(entity_b, entity_a);
  }
}

void Collision::on_collision(const CollisionEvent& c)
{
  auto& velocities = this->_registery.get().get_components<Velocity>();
  auto& positions = this->_registery.get().get_components<Position>();
  auto const& collidables = this->_registery.get().get_components<Collidable>();

  bool both_solid = this->_registery.get().has_component<Collidable>(c.a)
      && this->_registery.get().has_component<Collidable>(c.b)
      && collidables[c.a]->collision_type == CollisionType::Solid
      && collidables[c.b]->collision_type == CollisionType::Solid;

  if (both_solid) {
    double dt = this->_registery.get().clock().delta_seconds();
    if (this->_registery.get().has_component<Velocity>(c.a)
        && this->_registery.get().has_component<Position>(c.a))
    {
      positions[c.a]->x -=
          velocities[c.a]->speed_x * velocities[c.a]->dir_x * dt;
      positions[c.a]->y -=
          velocities[c.a]->speed_y * velocities[c.a]->dir_y * dt;
    }
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new Collision(r, e);
}
}
