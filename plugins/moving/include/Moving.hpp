#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

struct Position
{
  Position(float x, float y)
      : x(x)
      , y(y)
  {
  }

  double x;
  double y;
};

class Moving : public APlugin
{
public:
  Moving(Registery& r, EntityLoader& l)
      : APlugin(
          r,
          l,
          {"health"}, // depends on
          {COMP_INIT(position, init_pos)} // componend loader
      )
  {
    this->_registery.get().register_component<Position>();
    this->_registery.get().add_system<Position>(
        [](Registery&, const SparseArray<Position>& s)
        {
          for (auto&& [position] : Zipper(s)) {
            std::cout << "last\n";
          }
        }, 0);
    this->_registery.get().add_system<Position>(
        [](Registery&, const SparseArray<Position>& s)
        {
          for (auto&& [position] : Zipper(s)) {
            std::cout << "middle\n";
          }
        }, 1);
    this->_registery.get().add_system<Position>(
        [](Registery&, const SparseArray<Position>& s)
        {
          for (auto&& [position] : Zipper(s)) {
              std::cout << "first\n";
          }
        }, 2);
    this->_registery.get().add_system<Position>(
        [](Registery&, const SparseArray<Position>& s)
        {
          for (auto&& [position] : Zipper(s)) {
              std::cout << "middle2\n";
          }
        }, 1);
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
