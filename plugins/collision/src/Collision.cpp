#include <cctype>
#include <iostream>
#include <string>
#include <vector>

#include "Collision.hpp"

#include "Json/JsonParser.hpp"
#include "Logger.hpp"
#include "algorithm/QuadTreeCollision.hpp"
#include "ecs/Registery.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
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
    : APlugin(
          r, l, {"moving"}, {COMP_INIT(Collidable, Collidable, init_collision)})
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
  std::string type_text = type_str.value();
  std::transform(
      type_text.begin(), type_text.end(), type_text.begin(), ::tolower);

  if (type_text == "trigger") {
    type = CollisionType::Trigger;
  } else if (type_text == "solid") {
    type = CollisionType::Solid;
  } else if (type_text == "push") {
    type = CollisionType::Push;
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

  for (auto&& [i, position, collidable] : ZipperIndex(positions, collidables)) {
    if (!collidables[i]->is_active) {
      continue;
    }
    entities.push_back(ICollisionAlgorithm::CollisionEntity {
        .entity_id = i,
        .bounds = Rect {.x = position.pos.x,
                        .y = position.pos.y,
                        .width = collidable.width,
                        .height = collidable.height}});
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

  if (!this->_registery.get().has_component<Collidable>(c.a)
      || !this->_registery.get().has_component<Collidable>(c.b)
      || !this->_registery.get().has_component<Position>(c.a)
      || !this->_registery.get().has_component<Position>(c.b))
  {
    return;
  }

  CollisionType type_a = collidables[c.a]->collision_type;
  CollisionType type_b = collidables[c.b]->collision_type;

  if ((type_a != CollisionType::Solid && type_a != CollisionType::Push)
      || (type_b != CollisionType::Solid && type_b != CollisionType::Push))
  {
    return;
  }
  double dt = this->_registery.get().clock().delta_seconds();

  if (this->_registery.get().has_component<Velocity>(c.a))
  {
    Vector2D movement = (velocities[c.a]->direction * dt).normalize() * velocities[c.a]->speed;

    if (this->_registery.get().has_component<Velocity>(c.b) && type_a == CollisionType::Push) {
      positions[c.a]->pos -= movement;
      positions[c.b]->pos += movement;
    } else if (type_a == CollisionType::Solid) {
      Vector2D collision_normal = (positions[c.a]->pos - positions[c.b]->pos).normalize();
      double dot_product = movement.dot(collision_normal);

      if (dot_product < 0) {
        Vector2D perpendicular_vector = collision_normal * dot_product;
        positions[c.a]->pos -= perpendicular_vector;
      }
    } else {
      positions[c.a]->pos -= movement;
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
