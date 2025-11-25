#pragma once

#include <format>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "Events.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Position.hpp"

class Moving : public APlugin
{
public:
  Moving(Registery& r, EntityLoader& l)
      : APlugin(
            r,
            l,
            {},
            {COMP_INIT(Position, init_pos), COMP_INIT(Velocity, init_velocity)})
  {
    this->_registery.get().register_component<Position>();
    this->_registery.get().register_component<Velocity>();
    this->_registery.get().add_system<Position, Velocity>(
        [](Registery&,
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
        });
  }

private:
  void init_pos(Registery::Entity const entity, JsonVariant const& config)
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

  void init_velocity(Registery::Entity const entity, JsonVariant const& config)
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

  const std::vector<std::string> depends_on;
};
