#include <cctype>
#include <iostream>
#include <string>
#include <vector>

#include "Collision.hpp"

#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "algorithm/QuadTreeCollision.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Collidable.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/InteractionZone.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Speed.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/events/CollisionEvent.hpp"
#include "plugin/events/InteractionZoneEvent.hpp"
#include "plugin/events/LoggerEvent.hpp"

Collision::Collision(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin(
          "collision",
          r,
          em,
          l,
          {"moving"},
          {COMP_INIT(Collidable, Collidable, init_collision),
           COMP_INIT(InteractionZone, InteractionZone, init_interaction_zone)})
{
  REGISTER_COMPONENT(Collidable)
  REGISTER_COMPONENT(InteractionZone)

  _collision_algo = std::make_unique<QuadTreeCollision>(2.0, 2.0);

  if (!_collision_algo) {
    LOGGER("COLLISION", LogLevel::ERROR, "Error loading collision algorithm")
  }

  _registry.get().add_system([this](Registry& r) { this->collision_system(r); },
                             3);
  _registry.get().add_system(
      [this](Registry& r) { this->interaction_zone_system(r); }, 3);

  SUBSCRIBE_EVENT(CollisionEvent, { this->on_collision(event); })
}

void Collision::set_algorithm(std::unique_ptr<ICollisionAlgorithm> algo)
{
  _collision_algo = std::move(algo);
}

void Collision::init_collision(Ecs::Entity const& entity,
                               JsonObject const& obj)
{
  auto const& size = get_value<Collidable, Vector2D>(
      this->_registry.get(), obj, entity, "size");
  auto const& type_str = get_value<Collidable, std::string>(
      this->_registry.get(), obj, entity, "collision_type");

  if (!size || !type_str) {
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
  } else if (type_text == "bounce") {
    type = CollisionType::Bounce;
  }

  init_component<Collidable>(this->_registry.get(),
                             this->_event_manager.get(),
                             entity,
                             *size,
                             type,
                             true);
}

void Collision::init_interaction_zone(Ecs::Entity const& entity,
                                      JsonObject const& obj)
{
  auto const& radius = get_value<Collidable, double>(
      this->_registry.get(), obj, entity, "radius");

  if (!radius) {
    std::cerr << "Error loading InteractionZone: missing radius\n";
    return;
  }

  init_component<InteractionZone>(this->_registry.get(),
                                  this->_event_manager.get(),
                                  entity,
                                  radius.value());
}

void Collision::collision_system(Registry& r)
{
  if (!_collision_algo) {
    return;
  }

  std::vector<ICollisionAlgorithm::CollisionEntity> entities;

  for (auto&& [i, position, collidable] : ZipperIndex<Position, Collidable>(r))
  {
    if (!collidable.is_active) {
      continue;
    }
    entities.push_back(ICollisionAlgorithm::CollisionEntity {
        .entity_id = i,
        .bounds = Rect {.x = position.pos.x,
                        .y = position.pos.y,
                        .width = collidable.size.x,
                        .height = collidable.size.y}});
  }

  _collision_algo->update(entities);
  auto collisions = _collision_algo->detect_collisions(entities);

  for (auto const& collision : collisions) {
    std::size_t entity_a = collision.entity_a;
    std::size_t entity_b = collision.entity_b;

    this->_event_manager.get().emit<CollisionEvent>(entity_a, entity_b);
    this->_event_manager.get().emit<CollisionEvent>(entity_b, entity_a);
  }
}

void Collision::interaction_zone_system(Registry& r)
{
  if (!_collision_algo) {
    return;
  }

  auto const& positions = r.get_components<Position>();
  for (auto&& [i, position, zone] : ZipperIndex<Position, InteractionZone>(r)) {
    if (!zone.enabled) {
      continue;
    }

    Rect range {.x = position.pos.x,
                .y = position.pos.y,
                .width = zone.radius * 2,
                .height = zone.radius * 2};

    std::vector<ICollisionAlgorithm::CollisionEntity> candidates =
        _collision_algo->detect_range_collisions(range);
    std::vector<Ecs::Entity> detected_entities;
    detected_entities.reserve(candidates.size());

    for (const auto& candidate : candidates) {
      if (candidate.entity_id == i) {
        continue;
      }
      Vector2D distance = positions[candidate.entity_id]->pos - position.pos;

      if (distance.length() <= zone.radius) {
        detected_entities.push_back(candidate.entity_id);
      }
    }
    if (!detected_entities.empty()) {
      this->_event_manager.get().emit<InteractionZoneEvent>(
          i, zone.radius, detected_entities);
    }
  }
}

void Collision::on_collision(const CollisionEvent& c)
{
  auto& directions = this->_registry.get().get_components<Direction>();
  auto& speeds = this->_registry.get().get_components<Speed>();
  auto& positions = this->_registry.get().get_components<Position>();
  auto const& teams = this->_registry.get().get_components<Team>();
  auto const& collidables = this->_registry.get().get_components<Collidable>();

  if (this->_registry.get().has_component<Team>(c.a)
      && this->_registry.get().has_component<Team>(c.b)
      && teams[c.a]->name == teams[c.b]->name)
  {
    return;
  }

  if (!this->_registry.get().has_component<Collidable>(c.a)
      || !this->_registry.get().has_component<Collidable>(c.b)
      || !this->_registry.get().has_component<Position>(c.a)
      || !this->_registry.get().has_component<Position>(c.b))
  {
    return;
  }

  CollisionType type_a = collidables[c.a]->collision_type;
  CollisionType type_b = collidables[c.b]->collision_type;

  if ((type_a != CollisionType::Solid && type_a != CollisionType::Push
       && type_a != CollisionType::Bounce)
      || (type_b != CollisionType::Solid && type_b != CollisionType::Push
          && type_b != CollisionType::Bounce))
  {
    return;
  }
  double dt = this->_registry.get().clock().delta_seconds();

  if (this->_registry.get().has_component<Direction>(c.a)
      && this->_registry.get().has_component<Speed>(c.a))
  {
    Vector2D movement =
        (directions[c.a]->direction).normalize() * speeds[c.a]->speed * dt;
    Vector2D collision_normal =
        (positions[c.a]->pos - positions[c.b]->pos).normalize();

    if (this->_registry.get().has_component<Direction>(c.b)
        && this->_registry.get().has_component<Speed>(c.b)
        && type_a == CollisionType::Push)
    {
      positions[c.a]->pos -= movement;
      positions[c.b]->pos += movement;
      this->_event_manager.get().emit<ComponentBuilder>(
          c.b,
          this->_registry.get().get_component_key<Position>(),
          positions[c.b]->to_bytes());
    } else if (type_a == CollisionType::Solid) {
      double overlap_x =
          ((collidables[c.a]->size.x + collidables[c.b]->size.x) / 2.0)
          - std::abs(positions[c.a]->pos.x - positions[c.b]->pos.x);
      double overlapY =
          ((collidables[c.a]->size.y + collidables[c.b]->size.y) / 2.0)
          - std::abs(positions[c.a]->pos.y - positions[c.b]->pos.y);

      if (overlap_x > 0 && overlapY > 0) {
        Vector2D clean_normal(0, 0);
        double min_overlap = 0;
        if (overlap_x < overlapY) {
          clean_normal = {
              (positions[c.a]->pos.x > positions[c.b]->pos.x) ? 1.0 : -1.0, 0};
          min_overlap = overlap_x;
        } else {
          clean_normal = {
              0, (positions[c.a]->pos.y > positions[c.b]->pos.y) ? 1.0 : -1.0};
          min_overlap = overlapY;
        }
        double dot = movement.dot(clean_normal);
        if (dot < -0.0001) {
          Vector2D slide = movement - clean_normal * dot;
          positions[c.a]->pos = (positions[c.a]->pos - movement) + slide;
        }
        double correction_amount = std::max(min_overlap - 0.1, 0.0);
        positions[c.a]->pos += clean_normal * correction_amount;
      }
    } else if (type_a == CollisionType::Bounce) {
      double dot_product = directions[c.a]->direction.dot(collision_normal);
      Vector2D reflected_direction =
          directions[c.a]->direction - (collision_normal * (2.0 * dot_product));

      directions[c.a]->direction = reflected_direction.normalize();
      positions[c.a]->pos += collision_normal * 0.01;
    } else {
      positions[c.a]->pos -= movement;
    }

    this->_event_manager.get().emit<ComponentBuilder>(
        c.a,
        this->_registry.get().get_component_key<Position>(),
        positions[c.a]->to_bytes());
  }
}

extern "C"
{
void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new Collision(r, em, e);
}
}
