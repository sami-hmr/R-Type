#pragma once

#include <string>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"

class IPlugin
{
public:
  virtual ~IPlugin() = default;

  virtual void setComponent(Registery::Entity entity,
                            std::string const& key,
                            JsonVariant const&) = 0;
};
