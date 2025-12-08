#pragma once

#include <functional>
#include <optional>
#include <string>
#include <variant>

#include "EntityLoader.hpp"
#include "IPlugin.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/IPlugin.hpp"

#define COMP_INIT(comp_name, comp_type, method_name) \
  {#comp_name, \
   [this](size_t entity, const JsonVariant& config) \
   { \
    try { \
        JsonObject obj = std::get<JsonObject>(config); \
        this->method_name(entity, obj); \
        if (obj.contains("hook")) { \
          try { \
            std::string hook_name = std::get<std::string>(obj.at("hook").value); \
            this->_registry.get().register_hook<comp_type>(hook_name, entity); \
          } catch (...) { \
          } \
        } \
        } catch (std::bad_variant_access const &) { \
        std::cout << "Error initializing component \"" << #comp_name << "\": only JsonObjects are supported\n"; \
        return; \
    } \
   }}

class APlugin : public IPlugin
{
public:
  APlugin(Registry& registry,
          EntityLoader& loader,
          std::vector<std::string> const& depends_on,
          std::unordered_map<
              std::string,
              std::function<void(Registry::Entity, JsonVariant const&)>>
              components,
          std::optional<JsonObject> const& config = std::nullopt);

  void set_component(Registry::Entity entity,
                     std::string const& key,
                     JsonVariant const& config) override;

protected:
  const std::unordered_map<
      std::string,
      std::function<void(Registry::Entity, JsonVariant const&)>>
      components;
  std::reference_wrapper<Registry> _registry;
  std::reference_wrapper<EntityLoader> _loader;
  std::optional<JsonObject> _config;
};
