#pragma once
#include <memory>
#include <optional>
#include <string>

#include "CustomException.hpp"
#include "ecs/Registery.hpp"
#include "plugin/IPlugin.hpp"

class EntityLoader;

CUSTOM_EXCEPTION(NotExistingLib)
CUSTOM_EXCEPTION(LoaderNotExistingFunction)
CUSTOM_EXCEPTION(LoaderException)

template<typename Module>
concept lib = std::is_base_of_v<IPlugin, Module>;

template<lib Module>
class LibLoader
{
public:
  virtual ~LibLoader() = default;
  virtual std::unique_ptr<Module> get_instance(
      const std::string& entry_point,
      Registery& r,
      EntityLoader& e,
      std::optional<JsonObject> const& config) = 0;
};
