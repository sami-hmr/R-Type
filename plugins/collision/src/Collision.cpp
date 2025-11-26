#include <iostream>
#include <stdexcept>
#include <variant>
#include <vector>

#include "Collision.hpp"

#include "Logger.hpp"
#include "algorithm/QuadTreeCollision.hpp"
#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Collidable.hpp"

Collision::Collision(Registery& r, EntityLoader& l)
    : APlugin(r, l, {}, {COMP_INIT(Collidable, init_collision)})
{
  _registery.get().register_component<Collidable>();

  _collision_algo = std::make_unique<QuadTreeCollision>(1920.0, 1080.0);

  if (!_collision_algo) {
    LOGGER("COLLISION", LogLevel::ERROR, "Error loading collision algorithm")
  }

  _registery.get().add_system<Position, Collidable>(
      [this](
          Registery& r, SparseArray<Position> pos, SparseArray<Collidable> col)
      { this->collision_system(r, pos, col); });
}

void Collision::set_algorithm(std::unique_ptr<ICollisionAlgorithm> algo)
{
  _collision_algo = std::move(algo);
}

void Collision::init_collision(Registery::Entity const entity,
                               JsonVariant const& config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    double width = std::get<double>(obj.at("width").value);
    double height = std::get<double>(obj.at("height").value);

    this->_registery.get().emplace_component<Collidable>(entity, width, height);
  } catch (std::bad_variant_access const&) {
    throw BadComponentDefinition("expected JsonObject");
  } catch (std::out_of_range const&) {
    throw UndefinedComponentValue(
        R"(expected "width": double and "height": double)");
  }
}

void Collision::collision_system(Registery& /*r*/,
                                 SparseArray<Position> positions,
                                 SparseArray<Collidable> collidables)
{
  if (!_collision_algo) {
    return;
  }

  _collision_algo->update(positions, collidables);

  auto collisions = _collision_algo->detect_collisions(positions, collidables);

  for (auto const& collision : collisions) {
    std::cout << "Collision: entity " << collision.entity_a << " <-> entity "
              << collision.entity_b << "\n";
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new Collision(r, e);
}
}
