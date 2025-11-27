#pragma once

#include <functional>
#include <string>

#include "EntityLoader.hpp"
#include "IPlugin.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/IPlugin.hpp"

#define COMP_INIT(comp_name, method_name) \
  {#comp_name, \
   [this](size_t entity, const JsonVariant& config) \
   { this->method_name(entity, config); }}

/**
 * @brief Base implementation for plugins that manage components.
 *
 * Handles component registration and dependency loading.
 */
class APlugin : public IPlugin
{
public:
  /**
   * @brief Constructs a plugin with its dependencies and component handlers.
   *
   * @param registery The ECS registry.
   * @param loader Entity loader for plugin dependencies.
   * @param depends_on List of required plugin names.
   * @param components Map of component names to initialization functions.
   */
  APlugin(Registery& registery,
          EntityLoader& loader,
          std::vector<std::string> const& depends_on,
          std::unordered_map<
              std::string,
              std::function<void(Registery::Entity, JsonVariant const&)>>
              components);

  /**
   * @brief Applies a component configuration to an entity.
   */
  void set_component(Registery::Entity entity,
                     std::string const& key,
                     JsonVariant const& config) override;

protected:
  const std::unordered_map<
      std::string,
      std::function<void(Registery::Entity, JsonVariant const&)>>
      components;
  std::reference_wrapper<Registery> _registery;
  std::reference_wrapper<EntityLoader> _loader;
};
