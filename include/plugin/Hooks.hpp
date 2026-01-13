/**
 * @file Hooks.hpp
 * @brief Hook resolution and JSON value extraction functions
 *
 * This file provides helper functions to extract values from JSON configuration
 * while resolving hook references. Hooks enable JSON strings to reference
 * component field values using special syntax (# for dynamic hooks, % for
 * static hooks).
 *
 * @section hooks_overview Overview
 *
 * The hook system allows JSON configurations to reference runtime values:
 * - **Direct values**: `"speed": 10` - literal number
 * - **Static hooks**: `"speed": "%PlayerConfig:maxSpeed"` - read once
 * - **Dynamic bindings**: `"target": "#Enemy:pos"` - live updates
 * - **Nested objects**: `"sprite": {"texture": "player.png", ...}` - construct
 * from JSON
 *
 * @section hooks_syntax Hook Syntax
 *
 * @subsection hooks_static Static Hooks (%)
 *
 * Format: `"%ComponentName:fieldName"`
 *
 * - Prefix: % indicates static hook
 * - Resolution: Read value once during initialization
 * - Use case: Global configuration, template values
 * - Binding: No dynamic binding registered
 *
 * @code{.json}
 * {
 *   "entity": {
 *     "components": {
 *       "Velocity": {
 *         "speed": "%GameConfig:playerSpeed"  // Read global config
 *       }
 *     }
 *   }
 * }
 * @endcode
 *
 * @subsection hooks_dynamic Dynamic Hooks (#)
 *
 * Format: `"#ComponentName:fieldName"`
 *
 * - Prefix: # indicates dynamic hook
 * - Resolution: Read initial value and register binding
 * - Use case: Data binding between components
 * - Binding: Auto-updates when source changes
 *
 * @code{.json}
 * {
 *   "follower": {
 *     "components": {
 *       "Follower": {
 *         "target": "#Leader:pos"  // Tracks leader's position
 *       }
 *     }
 *   }
 * }
 * @endcode
 *
 * @subsection hooks_nested Nested Fields
 *
 * Access sub-fields using colon separator:
 *
 * @code{.json}
 * {
 *   "projectile": {
 *     "components": {
 *       "Velocity": {
 *         "speed.x": "#Player:velocity.speed.x",  // Just the x component
 *         "speed.y": "#Player:velocity.speed.y"
 *       }
 *     }
 *   }
 * }
 * @endcode
 *
 * @section hooks_functions Functions
 *
 * @subsection hooks_getref get_ref<T>()
 *
 * Retrieves a const reference to a value from JSON, resolving hooks if present.
 *
 * @code
 * JsonObject config = load_json("config.json");
 * Registry r;
 *
 * // Get direct value
 * auto speed_ref = get_ref<double>(r, config, "speed");
 * if (speed_ref) {
 *   double speed = speed_ref->get();  // Dereference reference_wrapper
 * }
 *
 * // Get hooked value
 * auto pos_ref = get_ref<Vector2D>(r, config, "position");
 * // If config["position"] = "#Player:pos", returns ref to Player::pos
 * @endcode
 *
 * @subsection hooks_getvaluecopy get_value_copy<T>()
 *
 * Retrieves a copy of a value, supporting hooks and JSON construction.
 *
 * @code
 * // Direct value
 * auto damage = get_value_copy<int>(r, event_obj, "damage");
 * // damage = 50
 *
 * // Hook reference
 * auto pos = get_value_copy<Vector2D>(r, obj, "position");
 * // If obj["position"] = "%Spawn:pos", reads Spawn::pos
 *
 * // JSON construction
 * auto rect = get_value_copy<Rect>(r, obj, "bounds");
 * // If obj["bounds"] = {"x": 0, "y": 0, "w": 100, "h": 50}
 * // Calls Rect(JsonObject) constructor
 * @endcode
 *
 * @subsection hooks_getvalue get_value<ComponentType, T>()
 *
 * Retrieves a value and optionally registers dynamic binding for auto-updates.
 *
 * @code
 * Registry::Entity entity = r.spawn_entity();
 *
 * // Static hook (no binding)
 * auto init_pos = get_value<Transform, Vector2D>(
 *   r, config, entity, "spawn_pos"
 * );
 *
 * // Dynamic binding (auto-updates)
 * auto target_pos = get_value<Follower, Vector2D>(
 *   r, config, entity, "target_pos"
 * );
 * // If config["target_pos"] = "#Leader:pos"
 * // Registers binding: Follower.target_pos <- Leader.pos
 * // Follower's target_pos updates when Leader moves
 * @endcode
 *
 * @subsection hooks_ishook is_hook()
 *
 * Checks if a JSON key contains a dynamic hook reference.
 *
 * @code
 * JsonObject obj;
 * obj["speed"] = 10;
 * obj["target"] = "#Enemy:pos";
 *
 * is_hook(obj, "speed");   // false - literal value
 * is_hook(obj, "target");  // true - starts with #
 * @endcode
 *
 * @section hooks_types Type Support
 *
 * @subsection hooks_json JSON-Compatible Types
 *
 * Direct value extraction supports JSON variant types:
 * - int
 * - double
 * - std::string
 * - bool
 * - JsonObject
 * - JsonArray
 *
 * @code
 * auto name = get_value_copy<std::string>(r, obj, "name");
 * auto health = get_value_copy<int>(r, obj, "health");
 * auto active = get_value_copy<bool>(r, obj, "active");
 * @endcode
 *
 * @subsection hooks_custom Custom Types via Constructors
 *
 * Types with JsonObject constructors are automatically supported:
 *
 * @code
 * struct Rect {
 *   double x, y, w, h;
 *
 *   Rect(JsonObject const& obj)
 *     : x(obj.at("x").value.get<double>())
 *     , y(obj.at("y").value.get<double>())
 *     , w(obj.at("w").value.get<double>())
 *     , h(obj.at("h").value.get<double>())
 *   {}
 * };
 *
 * // JSON: {"bounds": {"x": 10, "y": 20, "w": 100, "h": 50}}
 * auto bounds = get_value_copy<Rect>(r, obj, "bounds");
 * // Automatically calls Rect(JsonObject) constructor
 * @endcode
 *
 * @section hooks_examples Complete Examples
 *
 * @subsection hooks_enemy Enemy Configuration
 *
 * @code{.json}
 * // config.json
 * {
 *   "templates": {
 *     "EnemyDefaults": {
 *       "health": 100,
 *       "speed": 5.0
 *     }
 *   },
 *   "entities": {
 *     "boss": {
 *       "components": {
 *         "Health": {
 *           "max": "%EnemyDefaults:health",  // Static: read once
 *           "current": "%EnemyDefaults:health"
 *         },
 *         "Velocity": {
 *           "speed": 8.0,  // Override default
 *           "direction": {"x": 1, "y": 0}
 *         }
 *       }
 *     }
 *   }
 * }
 * @endcode
 *
 * @code
 * // Loading code
 * JsonObject config = load_json("config.json");
 * JsonObject boss_obj = config["entities"]["boss"]["components"]["Health"];
 *
 * auto max_health = get_value_copy<int>(r, boss_obj, "max");
 * // Resolves "%EnemyDefaults:health" -> 100
 * @endcode
 *
 * @subsection hooks_follower Follower AI
 *
 * @code{.json}
 * {
 *   "entities": {
 *     "player": {
 *       "id": 1,
 *       "components": {
 *         "Position": {"x": 100, "y": 200}
 *       }
 *     },
 *     "companion": {
 *       "id": 2,
 *       "components": {
 *         "Follower": {
 *           "target_pos": "#Player:pos",  // Dynamic binding!
 *           "offset": {"x": -20, "y": 0}
 *         }
 *       }
 *     }
 *   }
 * }
 * @endcode
 *
 * @code
 * // Component implementation
 * struct Follower {
 *   Vector2D target_pos;
 *   Vector2D offset;
 *
 *   Follower(Registry& r, JsonObject const& obj, Registry::Entity self)
 *     : target_pos(get_value<Follower, Vector2D>(
 *         r, obj, self, "target_pos"
 *       ).value_or(Vector2D{0, 0}))
 *     , offset(get_value_copy<Vector2D>(r, obj, "offset")
 *       .value_or(Vector2D{0, 0}))
 *   {}
 * };
 *
 * // After loading:
 * // - Companion's target_pos initially set to Player's position
 * // - Binding registered: Follower.target_pos <- Player.pos
 * // - When Player moves, Companion's target_pos auto-updates
 * @endcode
 *
 * @subsection hooks_weapon Weapon System
 *
 * @code{.json}
 * {
 *   "weapons": {
 *     "pistol": {
 *       "damage": 10,
 *       "fire_rate": 2.0,
 *       "projectile": {
 *         "components": {
 *           "Velocity": {
 *             "speed": {"x": "#Player:velocity.speed.x", "y": 0},
 *             "direction": {"x": 1, "y": 0}
 *           },
 *           "Damage": {
 *             "value": "%pistol:damage"  // Reference weapon config
 *           }
 *         }
 *       }
 *     }
 *   }
 * }
 * @endcode
 *
 * @section hooks_implementation Implementation Details
 *
 * @subsection hooks_resolution Resolution Process
 *
 * 1. **Check string prefix**:
 *    - `#` -> Dynamic hook (register binding)
 *    - `%` -> Static hook (read once)
 *    - Neither -> Try direct value or JSON construction
 *
 * 2. **Parse hook reference**:
 *    - Format: "ComponentName:fieldName"
 *    - Split on first `:` to get component and field
 *
 * 3. **Lookup value**:
 *    - Call Registry::get_hooked_value<T>(component, field)
 *    - Returns std::optional<std::reference_wrapper<const T>>
 *
 * 4. **Register binding** (dynamic hooks only):
 *    - Call Registry::register_binding<ComponentType, T>(entity, field, hook)
 *    - Creates live connection between source and target
 *
 * 5. **Return value**:
 *    - get_ref: Returns reference (optional<reference_wrapper>)
 *    - get_value_copy: Returns copy (optional<T>)
 *    - get_value: Returns copy + registers binding
 *
 * @subsection hooks_errors Error Handling
 *
 * All functions return std::optional:
 * - **Success**: optional contains value
 * - **Failure**: optional is empty (nullopt)
 *
 * Failure cases:
 * - Key not found in JSON object
 * - Hook reference invalid (bad format, component not found)
 * - Type mismatch (requested type != actual type)
 * - bad_variant_access, bad_any_cast, out_of_range
 *
 * @code
 * auto value = get_value_copy<int>(r, obj, "missing_key");
 * if (!value) {
 *   // Handle error: key not found or type mismatch
 *   int default_value = 0;
 * } else {
 *   int actual_value = value.value();
 * }
 *
 * // Or use value_or for defaults
 * int speed = get_value_copy<int>(r, obj, "speed").value_or(10);
 * @endcode
 *
 * @see HookMacros.hpp for HOOKABLE(), HOOK(), HOOK_CUSTOM() macros
 * @see HookConcept.hpp for hookable type requirements
 * @see Registry::get_hooked_value() for hook value retrieval
 * @see Registry::register_binding() for dynamic binding registration
 */

#pragma once

#include <any>
#include <cstddef>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/events/LogMacros.hpp"

/**
 * @struct is_in_json_variant
 * @brief Type trait to check if T is a valid JSON variant type
 *
 * Determines at compile-time whether a type can be directly stored in a
 * JsonValue variant. Used for SFINAE to enable direct value extraction.
 *
 * Supported types:
 * - int
 * - double
 * - std::string
 * - bool
 * - JsonObject
 * - JsonArray
 *
 * @tparam T Type to check
 *
 * @code
 * static_assert(is_in_json_variant_v<int>);        // true
 * static_assert(is_in_json_variant_v<double>);     // true
 * static_assert(is_in_json_variant_v<std::string>); // true
 * static_assert(!is_in_json_variant_v<Vector2D>);  // false
 * @endcode
 */
template<typename T>
struct is_in_json_variant
    : std::disjunction<std::is_same<T, int>,
                       std::is_same<T, double>,
                       std::is_same<T, std::string>,
                       std::is_same<T, bool>,
                       std::is_same<T, JsonObject>,
                       std::is_same<T, JsonArray>>
{
};

/**
 * @var is_in_json_variant_v
 * @brief Convenience variable template for is_in_json_variant
 *
 * @tparam T Type to check
 */
template<typename T>
inline constexpr bool is_in_json_variant_v = is_in_json_variant<T>::value;

/**
 * @brief Retrieves a const reference to a value from JSON, resolving hooks
 *
 * Attempts to extract a value of type T from a JSON object at the specified
 * key. If the value is a hook string (starts with # or %), resolves it to the
 * component field reference. Otherwise, returns a reference to the JSON value.
 *
 * @tparam T The expected type of the value
 * @param r Registry instance for hook resolution
 * @param object JSON object containing the key
 * @param key Key name to lookup
 *
 * @return Optional reference wrapper to the value, or nullopt on failure
 *
 * @details
 * Resolution priority:
 * 1. Check if value is a string starting with # or %
 * 2. If so, parse as "ComponentName:fieldName" and resolve via Registry
 * 3. If not a hook, try direct type extraction from JSON variant
 * 4. Return nullopt if key missing or type mismatch
 *
 * @note Returns a reference, not a copy - value lifetime tied to source
 * @note For hooks, reference points to component field (updated if dynamic)
 * @note For direct values, reference points to JSON storage (immutable)
 *
 * @code
 * JsonObject obj;
 * obj["speed"] = 10.5;
 *
 * auto ref = get_ref<double>(r, obj, "speed");
 * if (ref) {
 *   double speed = ref->get();  // 10.5
 * }
 * @endcode
 *
 * @code
 * JsonObject obj;
 * obj["max_health"] = "%PlayerConfig:baseHealth";
 *
 * // Assuming PlayerConfig component has baseHealth = 100
 * auto ref = get_ref<int>(r, obj, "max_health");
 * if (ref) {
 *   int health = ref->get();  // 100
 * }
 * @endcode
 *
 * @code
 * JsonObject obj;
 * obj["target"] = "#Enemy:pos";
 *
 * auto ref = get_ref<Vector2D>(r, obj, "target");
 * if (ref) {
 *   Vector2D pos = ref->get();  // Current enemy position
 *   // Note: reference may update if binding is registered elsewhere
 * }
 * @endcode
 */
template<typename T>
std::optional<std::reference_wrapper<const T>> get_ref(Registry& r,
                                                       JsonObject const& object,
                                                       std::string const& key)
{
  auto it = object.find(key);
  if (it == object.end()) {
    return std::nullopt;
  }

  auto const& value = it->second.value;

  try {
    std::string hook = std::get<std::string>(value);
    try {
      if (hook.starts_with('#')) {
        std::string striped = hook.substr(1);
        std::string comp = striped.substr(0, striped.find(':'));
        std::string value = striped.substr(striped.find(':') + 1);
        return r.get_hooked_value<T>(comp, value);
      }
    } catch (std::bad_any_cast const&) {
      LOGGER_EVTLESS(
          LogLevel::ERROR,
          "Hooks",
          std::format(R"(Error geting hooked value "{}": Invalid type\n)",
                      hook));
      return std::nullopt;
    } catch (std::out_of_range const&) {
      LOGGER_EVTLESS(
          LogLevel::ERROR,
          "Hooks",
          std::format(R"(Error geting hooked value "{}": Invalid hook\n)",
                      hook));
      return std::nullopt;
    }
  } catch (std::bad_variant_access const&) {  // NOLINT intentional fallthrought
  }

  if constexpr (is_in_json_variant_v<T>) {
    try {
      return std::reference_wrapper<const T>(std::get<T>(value));
    } catch (std::bad_variant_access const&) {
      return std::nullopt;
    }
  }

  return std::nullopt;
}

/**
 * @brief Gets a copy of a value from JSON, supporting hooks and construction
 *
 * Retrieves a value of type T from a JSON object, with support for:
 * - Direct JSON values (int, double, string, bool, object, array)
 * - Static hook references (% prefix)
 * - Dynamic hook references (# prefix) - reads value but doesn't bind
 * - JSON object construction (if T has JsonObject constructor)
 *
 * @tparam T The expected type of the value
 * @tparam Args Additional constructor arguments for JSON construction
 * @param r Registry instance for hook resolution
 * @param object JSON object containing the key
 * @param key Key name to lookup
 * @param args Additional arguments passed to T's constructor (if needed)
 *
 * @return Optional containing a copy of the value, or nullopt on failure
 *
 * @details
 * Resolution priority:
 * 1. Try get_ref<T>() for direct value or hook
 * 2. If successful, return a copy of the referenced value
 * 3. If T is constructible from JsonObject, try JSON construction
 * 4. Return nullopt if all methods fail
 *
 * @note Always returns a copy - caller owns the returned value
 * @note For static hooks only - dynamic binding requires get_value()
 *
 * @code
 * JsonObject obj;
 * obj["damage"] = 50;
 *
 * auto damage = get_value_copy<int>(r, obj, "damage");
 * // damage == optional<int>(50)
 * @endcode
 *
 * @code
 * JsonObject obj;
 * obj["speed"] = "%Config:playerSpeed";
 *
 * auto speed = get_value_copy<double>(r, obj, "speed");
 * // Reads Config::playerSpeed and returns a copy
 * @endcode
 *
 * @code
 * JsonObject obj;
 * obj["rect"] = JsonObject{
 *   {"x", 10}, {"y", 20}, {"w", 100}, {"h", 50}
 * };
 *
 * auto rect = get_value_copy<Rect>(r, obj, "rect");
 * // Calls Rect(JsonObject) constructor
 * @endcode
 *
 * @code
 * auto health = get_value_copy<int>(r, obj, "health").value_or(100);
 * // Returns 100 if "health" key missing or invalid
 * @endcode
 */
template<typename T, typename... Args>
std::optional<T> get_value_copy(Registry& r,
                                JsonObject const& object,
                                std::string const& key,
                                Args&&... args)
{
  auto tmp = get_ref<T>(r, object, key);
  if (tmp.has_value()) {
    return tmp.value().get();
  }

  if constexpr (std::is_constructible_v<T, JsonObject, Args...>) {
    try {
      const JsonObject& obj = std::get<JsonObject>(object.at(key).value);
      return T(obj, std::forward<Args>(args)...);
    } catch (std::bad_variant_access const&) {
      LOGGER_EVTLESS(LogLevel::ERROR,
                     "Hooks",
                     "hooked value construction via jsonobject failed");
    } catch (std::out_of_range const&) {
      LOGGER_EVTLESS(
          LogLevel::ERROR, "Hooks", "hooked value lookup in object failed");
    }
  }

  return std::nullopt;
}

/**
 * @brief Gets a value and registers a dynamic binding for auto-updates
 *
 * Retrieves a value from JSON with full hook support, including dynamic
 * binding registration. When the value is a dynamic hook (# prefix), this
 * function registers a binding that auto-updates the target component field
 * whenever the source changes.
 *
 * @tparam ComponentType The component type containing the field (for binding)
 * @tparam T The field value type
 * @tparam Args Additional constructor arguments
 * @param r Registry instance
 * @param object JSON object containing the key
 * @param entity Entity that owns the component (for binding registration)
 * @param field_name Name of the field in ComponentType (for binding
 * registration)
 * @param args Additional constructor arguments
 *
 * @return Optional containing the value, or nullopt on failure
 *
 * @details
 * Hook formats:
 * - **#self:Component:field**: Dynamic hook - same entity binding
 * - **#global:Name:field**: Dynamic hook - global singleton binding
 * - **%self:Component:field**: Static hook - read once from same entity
 * - **%global:Name:field**: Static hook - read once from global
 * - **No prefix**: Direct value or JSON construction
 *
 * Binding registration (# hooks only):
 * @code
 * r.register_binding<ComponentType, T>(
 *   entity,
 *   field_name,
 *   "scope:Component:field"
 * );
 * @endcode
 *
 * @warning This function should be called from component constructors
 * @note Use get_value_copy() if dynamic binding is not needed
 * @note Empty T{} returned if hook exists but no mapped entities found
 *
 * @code
 * struct Follower {
 *   Vector2D target_pos;
 *
 *   Follower(Registry& r, JsonObject const& obj, Registry::Entity self) {
 *     // JSON: "target_pos": "#self:Position:pos"
 *     target_pos = get_value<Follower, Vector2D>(
 *       r, obj, self, "target_pos"
 *     ).value_or(Vector2D{0, 0});
 *
 *     // Binding registered: Follower.target_pos <- self.Position.pos
 *     // target_pos will auto-update when Position changes
 *   }
 * };
 * @endcode
 *
 * @code
 * struct HealthBar {
 *   int max_health;
 *
 *   HealthBar(Registry& r, JsonObject const& obj, Registry::Entity self) {
 *     // JSON: "max_health": "#global:GameConfig:maxHealth"
 *     max_health = get_value<HealthBar, int>(
 *       r, obj, self, "max_health"
 *     ).value_or(100);
 *
 *     // Binding registered: HealthBar.max_health <-
 * global.GameConfig.maxHealth
 *   }
 * };
 * @endcode
 *
 * @code
 * struct Enemy {
 *   int max_health;
 *
 *   Enemy(Registry& r, JsonObject const& obj, Registry::Entity self) {
 *     // JSON: "max_health": "%global:EnemyDefaults:health"
 *     max_health = get_value<Enemy, int>(
 *       r, obj, self, "max_health"
 *     ).value_or(100);
 *
 *     // No binding registered - value read once
 *   }
 * };
 * @endcode
 */
template<typename ComponentType, typename T, typename... Args>
std::optional<T> get_value(Registry& r,
                           JsonObject const& object,
                           Registry::Entity entity,
                           std::string const& field_name,
                           Args&&... args)
{
  try {
    std::string value_str = std::get<std::string>(object.at(field_name).value);

    // self hook: @self
    if (value_str.starts_with('@')) {
      if constexpr (std::is_same_v<T, std::size_t>) {
        if (value_str.substr(1) == "self") {
          return entity;
        }
      }
    }
    // Dynamic hook: #scope:component:field
    if (value_str.starts_with('#')) {
      std::string stripped = value_str.substr(1);

      // Register binding for auto-updates
      r.template register_binding<ComponentType, T>(
          entity, field_name, stripped);

      // Parse scope:component:field
      size_t first_colon = stripped.find(':');
      size_t second_colon = stripped.find(':', first_colon + 1);

      if (first_colon == std::string::npos || second_colon == std::string::npos)
      {
        LOGGER_EVTLESS(
            LogLevel::ERROR,
            "Hooks",
            std::format(
                R"(Error parsing hook "{}": Expected format "scope:component:field")",
                value_str));
        return T {};
      }

      std::string scope = stripped.substr(0, first_colon);
      std::string comp =
          stripped.substr(first_colon + 1, second_colon - first_colon - 1);
      std::string value = stripped.substr(second_colon + 1);

      try {
        std::optional<std::reference_wrapper<T>> hooked_val;

        if (scope == "self") {
          // Same-entity hook: append entity ID
          std::string hook_key = comp + "{" + std::to_string(entity) + "}";
          hooked_val = r.template get_hooked_value<T>(hook_key, value);
        } else if (scope == "global") {
          // Global hook: use new method
          hooked_val = r.template get_global_hooked_value<T>(comp, value);
        } else {
          LOGGER_EVTLESS(
              LogLevel::ERROR,
              "Hooks",
              std::format(R"(Unknown scope "{}": Expected "self" or "global")",
                          scope));
        }

        if (hooked_val) {
          return hooked_val->get();
        }
      } catch (std::out_of_range const&) {
        LOGGER_EVTLESS(
            LogLevel::ERROR,
            "Hooks",
            std::format(
                R"(Error getting hooked value "{}": couldn't find the hook)",
                value_str));
      }
      return T {};
    }

    // Static hook: %scope:component:field
    if (value_str.starts_with('%')) {
      std::string stripped = value_str.substr(1);

      // Parse scope:component:field
      size_t first_colon = stripped.find(':');
      size_t second_colon = stripped.find(':', first_colon + 1);

      if (first_colon == std::string::npos || second_colon == std::string::npos)
      {
        LOGGER_EVTLESS(
            LogLevel::ERROR,
            "Hooks",
            std::format(
                R"(Error parsing hook "{}": Expected format "scope:component:field")",
                value_str));
        return std::nullopt;
      }

      std::string scope = stripped.substr(0, first_colon);
      std::string comp =
          stripped.substr(first_colon + 1, second_colon - first_colon - 1);
      std::string value = stripped.substr(second_colon + 1);

      std::optional<std::reference_wrapper<T>> hooked_val;

      if (scope == "self") {
        // Same-entity hook: append entity ID
        std::string hook_key = comp + "{" + std::to_string(entity) + "}";
        hooked_val = r.get_hooked_value<T>(hook_key, value);
      } else if (scope == "global") {
        // Global hook: use new method
        hooked_val = r.template get_global_hooked_value<T>(comp, value);
      } else {
        LOGGER_EVTLESS(
            LogLevel::ERROR,
            "Hooks",
            std::format(R"(Unknown scope "{}": Expected "self" or "global")",
                        scope));
      }

      if (hooked_val) {
        return hooked_val->get();
      }
    }

  } catch (std::bad_variant_access const&) {  // NOLINT intentional fallthrough
  }

  return get_value_copy<T>(r, object, field_name, std::forward<Args>(args)...);
}

/**
 * @brief Checks if a JSON value is a dynamic hook reference
 *
 * Determines whether a JSON object key contains a dynamic hook string
 * (starts with # character). Static hooks (% prefix) return false.
 *
 * @param object JSON object to check
 * @param key Key name to examine
 *
 * @return true if value is a string starting with #, false otherwise
 *
 * @note Only detects dynamic hooks (#), not static hooks (%)
 * @note Returns false for non-string values or missing keys
 *
 * @code
 * JsonObject obj;
 * obj["direct"] = 100;
 * obj["static"] = "%Config:value";
 * obj["dynamic"] = "#Player:pos";
 *
 * is_hook(obj, "direct");   // false - not a string
 * is_hook(obj, "static");   // false - % prefix, not #
 * is_hook(obj, "dynamic");  // true - # prefix
 * is_hook(obj, "missing");  // false - key doesn't exist
 * @endcode
 */
inline bool is_hook(JsonObject const& object, std::string const& key)
{
  try {
    std::string value = std::get<std::string>(object.at(key).value);
    return value.starts_with('#');
  } catch (std::bad_variant_access const&) {
    // Value is not a string
    return false;
  } catch (std::out_of_range const&) {
    // Key not found
    return false;
  }
}
