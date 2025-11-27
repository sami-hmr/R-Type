#pragma once

#include <string>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"

/**
 * @brief Interface for plugins that add components to entities.
 */
class IPlugin
{
public:
  virtual ~IPlugin() = default;

  /**
   * @brief Sets a component on an entity from configuration data.
   *
   * @param entity The target entity.
   * @param key Component identifier.
   * @param config JSON configuration for the component.
   */
  virtual void set_component(Registery::Entity entity,
                             std::string const& key,
                             JsonVariant const&) = 0;
};
