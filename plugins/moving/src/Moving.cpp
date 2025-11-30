#include "Moving.hpp"

#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"

void Moving::moving_system(Registery&,
                           SparseArray<Position>& positions,
                           const SparseArray<Velocity>& velocities)
{
  for (auto&& [position, velocity] : Zipper(positions, velocities)) {
    if (velocity.x != 0) {
      position.x += velocity.y;
    }
    if (velocity.y != 0) {
      position.y += velocity.y;
    }
  }
}

void Moving::init_pos(Registery::Entity const &entity, JsonVariant const& config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    double x = std::get<double>(obj.at("x").value);
    double y = std::get<double>(obj.at("y").value);
    this->_registery.get().emplace_component<Position>(entity, x, y);
  } catch (std::bad_variant_access const&) {
    throw BadComponentDefinition("expected JsonObject");
  } catch (std::out_of_range const&) {
    throw UndefinedComponentValue(R"(expected "x": double and "y": double )");
  }
}

void Moving::init_velocity(Registery::Entity const &entity, JsonVariant const& config)
{
  try {
    JsonObject obj = std::get<JsonObject>(config);
    double x = std::get<double>(obj.at("x").value);
    double y = std::get<double>(obj.at("y").value);
    this->_registery.get().emplace_component<Velocity>(entity, x, y);
  } catch (std::bad_variant_access const&) {
    throw BadComponentDefinition("expected JsonObject");
  } catch (std::out_of_range const&) {
    throw UndefinedComponentValue(R"(expected "x": double and "y": double )");
  }
}

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new Moving(r, e);
}
}
