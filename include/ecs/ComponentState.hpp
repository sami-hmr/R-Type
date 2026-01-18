#pragma once

#include <vector>

#include "plugin/Byte.hpp"

/**
 * @file ComponentState.hpp
 * @brief Component state serialization structure
 */

/**
 * @struct ComponentState
 * @brief Captures the state of a component type across all entities
 *
 * This structure is used for serialization and state snapshots of the ECS.
 * It stores all instances of a component type along with their entity
 * associations. Primarily used for network synchronization and save/load
 * functionality.
 *
 * @see Registry::get_state()
 */
struct ComponentState
{
  std::string id;  ///< Component type identifier (format: "plugin:component")
  std::vector<std::pair<size_t, ByteArray>>
      comps = {};  ///< Entity-component pairs (entity ID, serialized component data)
};
