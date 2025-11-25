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
      : APlugin(r, l, {}, {COMP_INIT(Position, init_pos)})
  {
    this->_registery.get().register_component<Position>();
    this->_registery.get().add_system<Position>(
        [this](Registery&, const SparseArray<Position>& s)
        {
          for (auto&& [position] : Zipper(s)) {
            LOGGER("SFML",
                   LogLevel::INFO,
                   std::format("x: {}, y: {}", position.x, position.y))
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

  const std::vector<std::string> depends_on;
};
