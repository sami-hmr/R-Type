/**
 * @file HookMacros.hpp
 * @brief Hook registration macros for component introspection
 *
 * This file provides macros to make component fields accessible at runtime
 * through a type-erased hook system. Hooks enable JSON configuration files and
 * dynamic bindings to reference component fields by string names (e.g.,
 * "Position:pos").
 *
 * @section hook_types Hook Types
 *
 * The system supports two types of hooks:
 *
 * 1. **Static Hooks** - Read-only references used during initialization
 *    - Syntax: `%ComponentName:field` in JSON
 *    - Use case: Reading global/template values
 *    - Example: `"speed": "%PlayerConfig:maxSpeed"`
 *
 * 2. **Dynamic Bindings** - Live references that auto-update on source changes
 *    - Syntax: `#ComponentName:field` in JSON
 *    - Use case: Data binding between components
 *    - Example: `"position": "#Target:pos"` (follows target)
 *
 * @section hook_usage Basic Usage
 *
 * @code
 * #include "plugin/HookMacros.hpp"
 *
 * struct Position {
 *   Vector2D pos;
 *   int z;
 *
 *   // Make both fields accessible as hooks
 *   HOOKABLE(Position, HOOK(pos), HOOK(z))
 * };
 *
 * struct Camera {
 *   Vector2D offset;
 *   float zoom;
 *
 *   // Custom hook names
 *   HOOKABLE(Camera,
 *     HOOK(offset),
 *     HOOK(zoom),
 *     HOOK_CUSTOM(viewOffset, offset)  // Alias "viewOffset" -> offset
 *   )
 * };
 * @endcode
 *
 * @section hook_json JSON Configuration
 *
 * @code{.json}
 * {
 *   "entity": {
 *     "components": {
 *       "Position": { "pos": [100, 50], "z": 1 },
 *       "Follower": {
 *         "target": "#Player:pos",     // Dynamic: tracks Player's position
 *         "offset": [10, 0]
 *       }
 *     }
 *   },
 *   "template": {
 *     "components": {
 *       "Enemy": {
 *         "speed": "%EnemyConfig:baseSpeed",  // Static: read once
 *         "health": 100
 *       }
 *     }
 *   }
 * }
 * @endcode
 *
 * @section hook_retrieval Hook Retrieval
 *
 * Hooks are accessed via Registry methods:
 *
 * @code
 * Registry r;
 *
 * // Get static hook value (read-only reference)
 * auto pos_ref = r.get_hooked_value<Vector2D>("Position", "pos");
 * if (pos_ref) {
 *   Vector2D current_pos = pos_ref->get();
 * }
 *
 * // Register dynamic binding (auto-updates)
 * r.register_binding<Follower, Vector2D>(
 *   follower_entity,
 *   "target_pos",    // Field name in Follower
 *   "Player:pos"     // Hook source
 * );
 * @endcode
 *
 * @section hook_nested Nested Field Access
 *
 * Hooks support nested fields via dot notation:
 *
 * @code
 * struct Velocity {
 *   Vector2D speed;
 *   Vector2D direction;
 *
 *   HOOKABLE(Velocity,
 *     HOOK(speed),
 *     HOOK(direction),
 *     HOOK(speed.x),      // Access Vector2D::x
 *     HOOK(speed.y),      // Access Vector2D::y
 *     HOOK(direction.x),
 *     HOOK(direction.y)
 *   )
 * };
 *
 * // In JSON: "#Velocity:speed.x" references just the x component
 * @endcode
 *
 * @see HookConcept.hpp for hookable type requirements
 * @see Hooks.hpp for get_ref() and get_value() helper functions
 * @see Registry::get_hooked_value() for runtime retrieval
 * @see Registry::register_binding() for dynamic bindings
 */

#pragma once

/**
 * @def HOOK_CUSTOM(key, var)
 * @brief Registers a component field with a custom hook name
 *
 * Creates a hook map entry that associates a string key with a component field.
 * The field is wrapped in a std::reference_wrapper and returned as std::any
 * for type-erased storage.
 *
 * @param key The string identifier for this hook (used in JSON:
 * "Component:key")
 * @param var The actual member variable to expose
 *
 * @note The key is stringified using #key, so no quotes needed
 *
 * @code
 * struct Transform {
 *   Vector2D position;
 *   float rotation;
 *
 *   HOOKABLE(Transform,
 *     HOOK_CUSTOM(pos, position),     // Hook "pos" -> position field
 *     HOOK_CUSTOM(rot, rotation)      // Hook "rot" -> rotation field
 *   )
 * };
 *
 * // JSON usage: "#Transform:pos" instead of "#Transform:position"
 * @endcode
 */
#define HOOK_CUSTOM(key, var) \
  {#key, \
   [](Component& self) -> std::any \
   { return std::reference_wrapper(self.var); }}
/**
 * @def HOOK(var)
 * @brief Registers a component field using its variable name as the hook key
 *
 * Shorthand for HOOK_CUSTOM where the hook name matches the field name.
 * This is the most common usage pattern.
 *
 * @param var The member variable to expose (name used as hook key)
 *
 * @code
 * struct Player {
 *   int health;
 *   int score;
 *
 *   HOOKABLE(Player,
 *     HOOK(health),  // Accessible as "Player:health"
 *     HOOK(score)    // Accessible as "Player:score"
 *   )
 * };
 * @endcode
 */
#define HOOK(var) HOOK_CUSTOM(var, var)

/**
 * @def HOOKABLE(type, ...)
 * @brief Makes a component type hookable by generating a static hook map
 *
 * This macro must be placed at the end of a component struct definition. It:
 * 1. Defines a Component type alias for use in hook lambdas
 * 2. Generates a static hook_map() method that returns the hook registry
 * 3. Satisfies the hookable concept requirement
 *
 * @param type The component type being made hookable
 * @param ... Comma-separated HOOK() or HOOK_CUSTOM() entries
 *
 * @implementation
 * The hook map is a static unordered_map<string, function<any(Component&)>>
 * where each entry maps a field name to a lambda that extracts that field
 * as a reference_wrapper<T> wrapped in std::any.
 *
 * @code
 * struct Position {
 *   double x;
 *   double y;
 *   int z;
 *
 *   HOOKABLE(Position, HOOK(x), HOOK(y), HOOK(z))
 * };
 *
 * // Hook map contains:
 * // "x" -> [](Position& p) { return std::ref(p.x); }
 * // "y" -> [](Position& p) { return std::ref(p.y); }
 * // "z" -> [](Position& p) { return std::ref(p.z); }
 * @endcode
 *
 * @code
 * struct Sprite {
 *   std::string texture;
 *   Rect source_rect;
 *   Color tint;
 *
 *   HOOKABLE(Sprite,
 *     HOOK(texture),
 *     HOOK(source_rect),
 *     HOOK(tint),
 *     HOOK_CUSTOM(rect, source_rect)  // Alias
 *   )
 * };
 * @endcode
 *
 * @note Empty hook lists are valid: HOOKABLE(MyType, ) creates an empty map
 * @note The type must be a complete type at the point of this macro invocation
 *
 * @see hookable concept in HookConcept.hpp
 */
#define HOOKABLE(type, ...) \
  using Component = type; \
  static const auto& hook_map() \
  { \
    static const std::unordered_map<std::string, \
                                    std::function<std::any(type&)>> \
        map {__VA_ARGS__}; \
    return map; \
  }
