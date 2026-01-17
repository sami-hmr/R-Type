#pragma once

#include <string>

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"

#ifdef _WIN32
#  define PLUGIN_EXPORT __declspec(dllexport)
#else
#  define PLUGIN_EXPORT
#endif

class IPlugin
{
public:
  virtual ~IPlugin() = default;

  virtual void set_component(Registry::Entity entity,
                             std::string const& key,
                             JsonVariant const&) = 0;
};
