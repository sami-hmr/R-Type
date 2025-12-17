/**
 * @file HookConcept.hpp
 * @brief Concept definition for hookable component types
 *
 * This file defines the hookable concept which constrains types to those that
 * provide a static hook_map() method. This concept is automatically satisfied
 * by any component using the HOOKABLE() macro.
 *
 * @section hook_purpose Purpose
 *
 * The hookable concept enables:
 * - **Compile-time verification** that types support runtime introspection
 * - **Generic programming** with hookable types (template constraints)
 * - **Safe hook access** via Registry methods
 *
 * @section hook_requirements Requirements
 *
 * A type T satisfies hookable if:
 * 1. T::hook_map() is a valid static method
 * 2. Returns const std::unordered_map<std::string,
 * std::function<std::any(T&)>>&
 * 3. The map associates string keys to field accessor functions
 *
 * @section hook_usage Usage
 *
 * @subsection hook_auto Automatic Satisfaction
 *
 * Most components automatically satisfy hookable via the HOOKABLE() macro:
 *
 * @code
 * #include "plugin/HookMacros.hpp"
 *
 * struct Transform {
 *   Vector2D position;
 *   float rotation;
 *   float scale;
 *
 *   // This macro makes Transform satisfy hookable
 *   HOOKABLE(Transform,
 *     HOOK(position),
 *     HOOK(rotation),
 *     HOOK(scale)
 *   )
 * };
 *
 * // Concept is satisfied - Transform is hookable
 * static_assert(hookable<Transform>);
 * @endcode
 *
 * @subsection hook_manual Manual Implementation
 *
 * You can manually implement hookable without macros:
 *
 * @code
 * #include "plugin/HookConcept.hpp"
 * #include <any>
 * #include <functional>
 * #include <string>
 * #include <unordered_map>
 *
 * struct CustomComponent {
 *   int value;
 *
 *   // Manually satisfy hookable
 *   static const auto& hook_map() {
 *     using Self = CustomComponent;
 *     static const std::unordered_map<std::string,
 *                                     std::function<std::any(Self&)>> map {
 *       {"value", [](Self& c) -> std::any {
 *         return std::reference_wrapper(c.value);
 *       }}
 *     };
 *     return map;
 *   }
 * };
 *
 * static_assert(hookable<CustomComponent>);
 * @endcode
 *
 * @subsection hook_constraint Template Constraints
 *
 * Use hookable to constrain template parameters:
 *
 * @code
 * // Only accept hookable types
 * template<hookable T>
 * void register_component(Registry& r, const std::string& name) {
 *   // Can safely access T::hook_map()
 *   for (const auto& [key, accessor] : T::hook_map()) {
 *     r.register_hook<T>(name, key, accessor);
 *   }
 * }
 *
 * // Also works with requires clause
 * template<typename T>
 *   requires hookable<T>
 * class ComponentWrapper {
 *   // Implementation can assume T has hook_map()
 * };
 * @endcode
 *
 * @subsection hook_detection Compile-Time Detection
 *
 * @code
 * struct Unhookable {
 *   int x;
 *   // No HOOKABLE() macro - concept not satisfied
 * };
 *
 * struct Hookable {
 *   int x;
 *   HOOKABLE(Hookable, HOOK(x))
 * };
 *
 * // Compile-time checks
 * static_assert(!hookable<Unhookable>);  // Fails - no hook_map()
 * static_assert(hookable<Hookable>);     // Passes
 *
 * // Conditional behavior
 * template<typename T>
 * void inspect(T& component) {
 *   if constexpr (hookable<T>) {
 *     std::cout << "Hookable with " << T::hook_map().size()
 *               << " fields\n";
 *   } else {
 *     std::cout << "Not hookable\n";
 *   }
 * }
 * @endcode
 *
 * @section hook_implementation Implementation Details
 *
 * The concept uses C++20 requires expression to check:
 * - **Expression validity**: T::hook_map() is callable
 * - **Return type**: Matches exact signature (const reference to map)
 * - **Map signature**: Maps strings to function<any(T&)>
 *
 * @code
 * template<typename T>
 * concept hookable = requires() {
 *   {
 *     T::hook_map()
 *   } -> std::same_as<const std::unordered_map<
 *                       std::string,
 *                       std::function<std::any(T&)>
 *                     >&>;
 * };
 * @endcode
 *
 * @section hook_registry Registry Integration
 *
 * The Registry uses hookable for safe hook access:
 *
 * @code
 * class Registry {
 * public:
 *   // Get static hook value (read-only)
 *   template<typename T, hookable Component>
 *   std::optional<std::reference_wrapper<const T>>
 *   get_hooked_value(const std::string& component_name,
 *                    const std::string& field_name) {
 *     // Can safely call Component::hook_map()
 *     const auto& hooks = Component::hook_map();
 *     auto it = hooks.find(field_name);
 *     if (it == hooks.end()) return std::nullopt;
 *
 *     // Get first instance and extract field
 *     auto& components = get_components<Component>();
 *     for (auto& opt : components) {
 *       if (opt.has_value()) {
 *         std::any result = it->second(opt.value());
 *         return std::any_cast<std::reference_wrapper<T>>(result);
 *       }
 *     }
 *     return std::nullopt;
 *   }
 * };
 * @endcode
 *
 * @section hook_examples Common Patterns
 *
 * @subsection hook_empty Empty Hook Map
 * @code
 * struct NoHooks {
 *   int internal_data;  // Not exposed
 *
 *   HOOKABLE(NoHooks, )  // Empty hook list is valid
 * };
 *
 * static_assert(hookable<NoHooks>);  // Still satisfies concept
 * // NoHooks::hook_map().empty() == true
 * @endcode
 *
 * @subsection hook_nested Nested Structures
 * @code
 * struct Inner {
 *   float value;
 *   HOOKABLE(Inner, HOOK(value))
 * };
 *
 * struct Outer {
 *   Inner inner;
 *
 *   HOOKABLE(Outer,
 *     HOOK(inner),        // Whole inner object
 *     HOOK(inner.value)   // Direct access to nested field
 *   )
 * };
 *
 * // Both are hookable
 * static_assert(hookable<Inner>);
 * static_assert(hookable<Outer>);
 * @endcode
 *
 * @see HookMacros.hpp for HOOKABLE(), HOOK(), and HOOK_CUSTOM() macros
 * @see Hooks.hpp for get_ref() and get_value() helper functions
 * @see Registry::get_hooked_value() for runtime hook access
 */

#pragma once

#include <any>
#include <concepts>
#include <functional>
#include <string>
#include <unordered_map>

/**
 * @concept hookable
 * @brief Requires a type to provide a static hook_map() method
 *
 * A type T is hookable if it defines a static method hook_map() that returns
 * a const reference to an unordered_map mapping field names (strings) to
 * accessor functions that extract those fields as std::any.
 *
 * @tparam T The type to check for hookability
 *
 * @details
 * Required signature:
 * @code
 * static const std::unordered_map<std::string,
 *                                 std::function<std::any(T&)>>&
 * T::hook_map();
 * @endcode
 *
 * The returned map should:
 * - Map field names (strings) to accessor lambdas
 * - Each lambda takes a T& and returns the field wrapped in std::any
 * - Fields are wrapped in std::reference_wrapper for reference semantics
 *
 * @note This concept is automatically satisfied by using HOOKABLE() macro
 * @note Empty hook maps (no fields exposed) are valid
 *
 * @example Basic Check
 * @code
 * struct Position {
 *   double x, y;
 *   HOOKABLE(Position, HOOK(x), HOOK(y))
 * };
 *
 * if constexpr (hookable<Position>) {
 *   // Position provides runtime introspection
 *   auto& hooks = Position::hook_map();
 *   std::cout << "Position has " << hooks.size() << " hooks\n";
 * }
 * @endcode
 *
 * @example Template Constraint
 * @code
 * template<hookable T>
 * void print_hooks() {
 *   for (const auto& [name, accessor] : T::hook_map()) {
 *     std::cout << "Hook: " << name << "\n";
 *   }
 * }
 *
 * print_hooks<Position>();  // OK - Position is hookable
 * print_hooks<int>();       // Error - int is not hookable
 * @endcode
 */
template<typename T>
concept hookable = requires() {
  {
    T::hook_map()
  } -> std::same_as<
      const std ::unordered_map<std ::string, std ::function<std ::any(T&)>>&>;
};
