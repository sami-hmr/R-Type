#pragma once

#include <cstdint>
#include <string>

#include "ByteParser/ByteParser.hpp"
#include "TwoWayMap.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

/**
 * @file Scenes.hpp
 * @brief Scene management system for organizing entities into layers
 */

/**
 * @enum SceneState
 * @brief Defines the activation state of a scene
 *
 * Scenes can be in different states controlling their visibility and update
 * behavior:
 * - ACTIVE: Scene is active and entities are processed by systems
 * - INACTIVE: Scene exists but is not processed or rendered
 */
enum class SceneState : std::uint8_t
{
  ACTIVE,  ///< Scene is active, entities are processed
  INACTIVE  ///< Scene is inactive, entities are not processed
};

/**
 * @brief Bidirectional mapping between SceneState enum and string
 * representations
 *
 * Used for JSON configuration parsing and serialization.
 * Allows conversion between SceneState enum values and their string
 * representations.
 */
static const TwoWayMap<SceneState, std::string> SCENE_STATE_STR = {
    {SceneState::ACTIVE, "active"},
    {SceneState::INACTIVE, "inactive"},
};

/**
 * @struct Scene
 * @brief Component that associates an entity with a specific scene
 *
 * The Scene component is used to organize entities into logical layers or
 * contexts. Only entities in currently active scenes are processed by systems
 * (via Zipper filtering). This enables multi-scene support for menus, HUDs,
 * gameplay layers, etc.
 *
 * Scene state is managed by the Registry, not the component itself. Each entity
 * is tagged with a scene name, and the Registry determines which scenes are
 * currently active.
 *
 * @note This component is automatically added to entities during
 * EntityLoader::load_entity()
 * @note Systems iterate only over entities in active scenes through the Zipper
 * pattern
 * @note Scene state lives in Registry::_scenes, not in this component
 *
 * @see Registry::add_scene()
 * @see Registry::activate_scene()
 * @see Zipper (filters entities by active scenes)
 */
struct Scene
{
  Scene() = default;

  /**
   * @brief Constructs a Scene component
   * @param scene_name Name identifier for the scene
   */
  explicit Scene(std::string scene_name)
      : scene_name(std::move(scene_name))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      Scene,
      ([](std::vector<char> name)
       { return Scene(std::string(name.begin(), name.end())); }),
      parseByteArray(parseAnyChar()))
  DEFAULT_SERIALIZE(string_to_byte(this->scene_name))

  CHANGE_ENTITY_DEFAULT

  HOOKABLE(Scene, HOOK(scene_name))

  std::string scene_name;  ///< Identifier for this scene
};
