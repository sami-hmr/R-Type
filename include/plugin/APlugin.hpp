#pragma once

#include <functional>
#include <optional>
#include <string>

#include "EntityLoader.hpp"
#include "IPlugin.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "ecs/EventManager.hpp"

#include "plugin/IPlugin.hpp"

#define COMP_INIT(comp_name, comp_type, method_name) \
  { \
    #comp_name, [this](size_t entity, const JsonVariant& config) \
    { \
      try { \
        JsonObject obj = std::get<JsonObject>(config); \
        this->method_name(entity, obj); \
        if (obj.contains("hook")) { \
          try { \
            std::string hook_name = \
                std::get<std::string>(obj.at("hook").value); \
            this->_registry.get().register_hook<comp_type>(hook_name, entity); \
          } catch (...) { \
          } \
        } \
      } catch (std::bad_variant_access const&) { \
        std::cout << "Error initializing component \"" << #comp_name \
                  << "\": only JsonObjects are supported\n"; \
        return; \
      } \
    } \
  }

#define REGISTER_COMPONENT(comp) \
  this->_registry.get().register_component<comp>( \
      std::format("{}:{}", this->name, #comp));

#define SUBSCRIBE_EVENT_PRIORITY(event_name, function, priority) \
this->_event_manager.get().on<event_name>( \
    #event_name, [this]([[maybe_unused]] event_name const& event) function, priority);


#define SUBSCRIBE_EVENT(event_name, function) SUBSCRIBE_EVENT_PRIORITY(event_name, function, 1)

class APlugin : public IPlugin
{
public:
  APlugin(
      std::string name,
      Registry& registry,
      EventManager &event_manager,
      EntityLoader& loader,
      std::vector<std::string> const& depends_on,
      std::unordered_map<
          std::string,
          std::function<void(Registry::Entity, JsonVariant const&)>> components,
      std::optional<JsonObject> const& config = std::nullopt);

  void set_component(Registry::Entity entity,
                     std::string const& key,
                     JsonVariant const& config) override;

protected:
  const std::string name;
  const std::unordered_map<
      std::string,
      std::function<void(Registry::Entity, JsonVariant const&)>>
      components;
  std::reference_wrapper<Registry> _registry;
  std::reference_wrapper<EventManager> _event_manager;
  std::reference_wrapper<EntityLoader> _loader;
  std::optional<JsonObject> _config;
};
