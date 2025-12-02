#pragma once

#include <format>
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
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"
#include "plugin/events/Events.hpp"

class Moving : public APlugin
{
public:
  Moving(Registery& r, EntityLoader& l);

private:
  void init_pos(Registery::Entity const& entity, JsonVariant const& config);
  void init_velocity(Registery::Entity const& entity,
                     JsonVariant const& config);

  void moving_system(Registery&,
                     SparseArray<Position>& positions,
                     const SparseArray<Velocity>& velocities);
};
