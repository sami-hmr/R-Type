#pragma once

#include <optional>

#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class RtypeSingle : public APlugin
{
public:
  RtypeSingle(Registry& r,
              EventManager& em,
              EntityLoader& l,
              std::optional<JsonObject> const& config);
  ~RtypeSingle() override = default;

private:
  std::optional<Registry::Entity> _player_entity;
};
