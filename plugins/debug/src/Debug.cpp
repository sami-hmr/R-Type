#include <iostream>
#include <typeindex>

#include "Debug.hpp"

#include "TypeFormatter.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Collidable.hpp"
#include "plugin/components/Health.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Speed.hpp"

Debug::Debug(Registry& r,
             EventManager& em,
             EntityLoader& l,
             std::optional<JsonObject> const& config)
    : APlugin("debug", r, em, l, {}, {}, config)
    , _enabled(true)
{
  // Initialize type formatters on first use
  initialize_type_formatters();

  // Register the debug system to run each frame
  _registry.get().add_system([this](Registry& r) { print_all_entities(r); });
}

void Debug::print_entity_info(Registry::Entity /*entity*/) {}

/**
 * @brief Helper template to print component data for a specific entity
 */
template<typename ComponentType>
void print_component_data(Registry& r,
                          size_t entity_id,
                          std::string const& component_name,
                          TypeFormatterRegistry const& formatter)
{
  auto& components = r.get_components<ComponentType>();

  if (components.size() <= entity_id || !components[entity_id].has_value()) {
    return;
  }

  std::cout << "  " << component_name << ":\n";

  // Get the hook map for this component type
  auto const& hooks = ComponentType::hook_map();

  // Iterate through all hooked fields
  for (auto const& [field_name, accessor] : hooks) {
    try {
      // Call the accessor to get the value as std::any
      std::any field_value = accessor(components[entity_id].value());

      // Get the type_index from the std::any
      std::type_index type_idx = field_value.type();

      // Format the value using our type registry
      std::string formatted_value = formatter.format(field_value, type_idx);

      std::cout << "    " << field_name << ": " << formatted_value << "\n";
    } catch (std::exception const& e) {
      std::cout << "    " << field_name << ": <error: " << e.what() << ">\n";
    }
  }
}

void Debug::print_all_entities(Registry& r)
{
  if (!_enabled) {
    return;
  }

  auto const& formatter = TypeFormatterRegistry::instance();

  std::cout << "\n========== DEBUG: Entity State ==========\n";

  for (size_t entity_id = 0; entity_id < 10; ++entity_id) {
    std::cout << "Entity " << entity_id << ":\n";

    print_component_data<Position>(r, entity_id, "Position", formatter);
    print_component_data<Health>(r, entity_id, "Health", formatter);
    print_component_data<Speed>(r, entity_id, "Speed", formatter);
    print_component_data<Collidable>(r, entity_id, "Collidable", formatter);
  }

  std::cout << "=========================================\n\n";

  // Disable after first print to avoid spam (you can change this behavior)
  _enabled = false;
}

extern "C"
{
void* entry_point(Registry& r,
                  EventManager& em,
                  EntityLoader& e,
                  std::optional<JsonObject> const& config)
{
  return new Debug(r, em, e, config);
}
}
