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
#include "plugin/components/Velocity.hpp"

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
        [this](Registery& r,
               SparseArray<Position>& pos,
               const SparseArray<Velocity>& vel)
        { this->moving_system(r, pos, vel); });
  }

private:
  void init_pos(Registery::Entity const entity, JsonVariant const& config);
  void init_velocity(Registery::Entity const entity, JsonVariant const& config);

  void moving_system(Registery&,
                     SparseArray<Position>& positions,
                     const SparseArray<Velocity>& velocities);

  const std::vector<std::string> depends_on;
};
