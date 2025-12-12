#pragma once

#include <concepts>
#include <cstddef>
#include <unordered_map>

#include "Json/JsonParser.hpp"
#include "plugin/Byte.hpp"
#include "TwoWayMap.hpp"

// template<typename K, typename V>
// class TwoWayMap;

class Registry;

template<typename T>
concept json_buildable = requires(Registry& r, JsonObject const& j) {
  { T(r, j) } -> std::same_as<T>;
};

/**
 * @brief Checks if a type has a change_entity method for mapping entity IDs
 * @details Requires a change_entity method that maps entities using a
 * TwoWayMap. The method must be const, return the same type, and accept a
 *          TwoWayMap<size_t, size_t> (compatible with Registry::Entity).
 *          Use this for events containing Registry::Entity fields.
 *
 * @example
 * struct MyEvent {
 *   Registry::Entity actor;
 *   auto change_entity(TwoWayMap<Registry::Entity, Registry::Entity> const&)
 * const -> MyEvent;
 * };
 * static_assert(EventHasChangeEntity<MyEvent>);
 */
template<typename T>
concept entity_convertible =
    requires(T const& event, std::unordered_map<std::size_t, std::size_t> const& map) {
      { event.change_entity(map) } -> std::same_as<T>;
    };
