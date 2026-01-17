#pragma once

#include <string>

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"

class IPlugin
{
public:
  virtual ~IPlugin() = default;

  virtual void set_component(Ecs::Entity entity,
                             std::string const& key,
                             JsonVariant const&) = 0;
};
