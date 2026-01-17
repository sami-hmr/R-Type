#pragma once

/**
 * @brief Maps a vector of Ecs::Entity using a TwoWayMap
 * @details Creates a new vector with all entities mapped.
 *
 * @code
 * DEFINE_CHANGE_ENTITY(
 *   result.source = map.at(source);
 *   result.candidates = MAP_ENTITY_VECTOR(candidates);
 * )
 * @endcode
 */
#define MAP_ENTITY_VECTOR(vec) \
  [&]() \
  { \
    auto mapped_vec = vec; \
    std::transform(mapped_vec.begin(), \
                   mapped_vec.end(), \
                   mapped_vec.begin(), \
                   [&](auto const& e) { return map.at(e); }); \
    return mapped_vec; \
  }()

/**
 * @brief Generates a change_entity method for event structs
 * @details Creates a const method that maps Ecs::Entity fields using a
 * TwoWayMap. Non-entity fields are automatically copied from the original
 * event. If any mapping fails (std::out_of_range), returns the original event
 * unchanged.
 *
 * @param ... Statements to modify entity fields (e.g., result.field =
 * map.at_second(field))
 *
 * @code
 * struct MyEvent {
 *   Ecs::Entity actor;
 *   Ecs::Entity target;
 *   int value;
 *
 *   DEFINE_CHANGE_ENTITY(
 *     result.actor = map.at(actor);
 *     result.target = map.at(target);
 *   )
 * };
 * @endcode
 */
#define CHANGE_ENTITY(...) \
  auto change_entity( \
      std::unordered_map<Ecs::Entity, Ecs::Entity> const& map) const \
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
  auto change_entity(std::unordered_map<std::size_t, std::size_t> const&) \
      const \
  { \
    return *this; \
  }
