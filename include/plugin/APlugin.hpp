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

class APlugin : public IPlugin
{
public:
  APlugin(Registery& registery,
          EntityLoader& loader,
          std::vector<std::string> const& depends_on,
          std::unordered_map<
              std::string,
              std::function<void(Registery::Entity, JsonVariant const&)>>
              components);

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
