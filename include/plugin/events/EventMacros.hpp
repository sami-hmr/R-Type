#pragma once

/**
 * @brief Maps a vector of Registry::Entity using a TwoWayMap
 * @details Creates a new vector with all entities mapped.
 *
 * @example
 * DEFINE_CHANGE_ENTITY(
 *   result.source = map.at_second(source);
 *   result.candidates = MAP_ENTITY_VECTOR(candidates);
 * )
 */
#define MAP_ENTITY_VECTOR(vec) \
  [&]() \
  { \
    auto mapped_vec = vec; \
    std::transform(mapped_vec.begin(), \
                   mapped_vec.end(), \
                   mapped_vec.begin(), \
                   [&](auto const& e) { return map.at_second(e); }); \
    return mapped_vec; \
  }()

/**
 * @brief Generates a change_entity method for event structs
 * @details Creates a const method that maps Registry::Entity fields using a
 * TwoWayMap. Non-entity fields are automatically copied from the original
 * event. If any mapping fails (std::out_of_range), returns the original event
 * unchanged.
 *
 * @param ... Statements to modify entity fields (e.g., result.field =
 * map.at_second(field))
 *
 * @example
 * struct MyEvent {
 *   Registry::Entity actor;
 *   Registry::Entity target;
 *   int value;
 *
 *   DEFINE_CHANGE_ENTITY(
 *     result.actor = map.at_second(actor);
 *     result.target = map.at_second(target);
 *   )
 * };
 */
#define CHANGE_ENTITY(...) \
  auto change_entity(TwoWayMap<Registry::Entity, Registry::Entity> const& map) \
      const \
  { \
    try { \
      auto result = *this; \
      __VA_ARGS__; \
      return result; \
    } catch (std::out_of_range const&) { \
      return *this; \
    } \
  }

#define CHANGE_ENTITY_DEFAULT \
  auto change_entity(TwoWayMap<std::size_t, std::size_t> const&) \
      const \
  { \
    return *this; \
  }
