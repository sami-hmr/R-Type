#pragma once

#include <stdexcept>
#include <string>
#include <utility>

#include "NetworkShared.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/events/LoggerEvent.hpp"

/**
 * @file InitComponent.hpp
 * @brief Component initialization helpers with automatic network
 * synchronization
 *
 * These functions wrap Registry::add_component() and
 * Registry::emplace_component() to automatically broadcast component additions
 * via ComponentBuilder events. This ensures component state is synchronized
 * across the network.
 */

/**
 * @brief Initializes a component on an entity with network synchronization
 *
 * Adds a pre-constructed component to an entity and emits a ComponentBuilder
 * event to synchronize the component state across the network.
 *
 * @tparam Component Component type (must satisfy component concept)
 * @param r Registry instance
 * @param to Target entity ID
 * @param comp Component instance to add (moved into storage)
 * @return Reference to the added component in the registry
 *
 * @note Emits ComponentBuilder for network synchronization
 * @note If component type is not registered, logs an error via LogEvent
 *
 * @see Registry::add_component()
 * @see ComponentBuilder
 */
template<component Component>
typename SparseArray<Component>::Ref init_component(Registry& r,
                                                    EventManager& em,
                                                    Ecs::Entity to,
                                                    Component comp)
{
  try {
    em.emit<ComponentBuilder>(ComponentBuilder(
        to, r.get_component_key<Component>(), comp.to_bytes()));
  } catch (std::out_of_range const&) {
    em.emit<LogEvent>("init", LogLevel::ERR, "unknow component");
  }
  return r.add_component<Component>(to, std::move(comp));
}

/**
 * @brief Constructs and initializes a component with network synchronization
 *
 * Emplaces a component on an entity by forwarding construction arguments,
 * and emits a ComponentBuilder event for network synchronization.
 *
 * @tparam Component Component type (must satisfy component concept)
 * @tparam Args Constructor argument types
 * @param r Registry instance
 * @param to Target entity ID
 * @param args Arguments forwarded to component constructor
 * @return Reference to the emplaced component in the registry
 *
 * @note More efficient than constructing separately then adding
 * @note Emits ComponentBuilder for network synchronization
 * @note Logs error if component type is not registered
 *
 * @see Registry::emplace_component()
 * @see ComponentBuilder
 */
template<component Component, typename... Args>
typename SparseArray<Component>::Ref init_component(Registry& r,
                                                    EventManager& em,
                                                    Ecs::Entity to,
                                                    Args... args)
{
  try {
    em.emit<ComponentBuilder>(ComponentBuilder(
        to, r.get_component_key<Component>(), Component(args...).to_bytes()));
  } catch (std::out_of_range const&) {
    em.emit<LogEvent>("init", LogLevel::ERR, "unknow component");
  }
  return r.emplace_component<Component>(to, std::forward<Args>(args)...);
}

/**
 * @brief Initializes a component from serialized data
 *
 * Deserializes and adds a component from a byte array, typically used for
 * network synchronization or save/load functionality. The component type
 * is identified by a string ID.
 *
 * @param r Registry instance
 * @param to Target entity ID
 * @param id Component type identifier (format: "plugin:component")
 * @param comp Serialized component data
 *
 * @note This overload is used by networking code to apply remote component
 * changes
 * @note Emits ComponentBuilder for further network propagation
 * @note Logs error if component ID is not recognized
 *
 * @see Registry::emplace_component(Entity, string, ByteArray)
 * @see ComponentBuilder
 */
inline void init_component(Registry& r,
                           EventManager& em,
                           Ecs::Entity to,
                           std::string const& id,
                           ByteArray const& comp)
{
  try {
    em.emit<ComponentBuilder>(ComponentBuilder(to, id, comp));
  } catch (std::out_of_range const&) {
    em.emit<LogEvent>("init", LogLevel::ERR, "unknow component");
  }
  r.emplace_component(to, id, comp);
}
