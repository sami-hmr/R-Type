#pragma once
#include <memory>
#include <string>

#include "CustomException.hpp"
#include "ecs/Registery.hpp"
#include "plugin/IPlugin.hpp"

class EntityLoader;

CUSTOM_EXCEPTION(NotExistingLib)
CUSTOM_EXCEPTION(LoaderNotExistingFunction)
CUSTOM_EXCEPTION(LoaderException)

/**
 * @brief Concept ensuring a module type derives from IPlugin.
 */
template<typename Module>
concept lib = std::is_base_of_v<IPlugin, Module>;

/**
 * @brief Interface for loading plugin modules dynamically.
 *
 * @tparam Module The plugin type to load, must satisfy lib concept.
 */
template<lib Module>
class LibLoader
{
public:
  virtual ~LibLoader() = default;

  /**
   * @brief Loads and returns an instance of the plugin module.
   *
   * @param entry_point Name of the factory function to call.
   * @param r Registry reference to pass to the plugin.
   * @param e EntityLoader reference to pass to the plugin.
   */
  virtual std::unique_ptr<Module> get_instance(const std::string& entry_point,
                                               Registery& r,
                                               EntityLoader& e) = 0;
};
