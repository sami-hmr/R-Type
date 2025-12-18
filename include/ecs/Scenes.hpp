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
 * - ACTIVE: Scene is active but not the main scene (overlay, UI layer, etc.)
 * - MAIN: Primary active scene (typically the gameplay scene)
 * - DISABLED: Scene exists but is not processed or rendered
 */
enum class SceneState : std::uint8_t
{
  ACTIVE,  ///< Scene is active but not primary
  MAIN,  ///< Primary active scene
  DISABLED  ///< Scene is disabled
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
    {SceneState::MAIN, "main"},
    {SceneState::DISABLED, "disabled"},
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
 * @note This component is automatically added to entities during
 * EntityLoader::load_entity()
 * @note Systems iterate only over entities in active scenes through the Zipper
 * pattern
 *
 * @see Registry::add_scene()
 * @see Registry::set_current_scene()
 * @see Zipper (filters entities by active scenes)
 */
struct Scene
{
  Scene() = default;

  /**
   * @brief Constructs a Scene component
   * @param scene_name Name identifier for the scene
   * @param state Activation state of the scene
   */
  Scene(std::string scene_name, SceneState state)
      : scene_name(std::move(scene_name))
      , state(state)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      Scene,
      ([](std::vector<char> name, SceneState state)
       { return Scene(std::string(name.begin(), name.end()), state); }),
      parseByteArray(parseAnyChar()),
      parseByte<SceneState>())
  DEFAULT_SERIALIZE(string_to_byte(this->scene_name),
                    type_to_byte((std::uint8_t)this->state))

  CHANGE_ENTITY_DEFAULT

  HOOKABLE(Scene, HOOK(scene_name), HOOK(state))

  std::string scene_name;  ///< Identifier for this scene
  SceneState state;  ///< Current activation state
};
