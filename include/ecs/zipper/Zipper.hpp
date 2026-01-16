#pragma once

#include <algorithm>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/SparseArray.hpp"

template<class... Comps>
class ZipperIndexIterator;

/**
 * @class ZipperIterator
 * @brief Iterator that simultaneously traverses multiple Comps and yields
 * tuples of their elements.
 * @tparam Comps Variadic template parameter pack of Comp types to
 * iterate over.
 *
 * This iterator implements a zipper pattern that combines multiple Comps
 * into a single iteration stream. It only yields values where all Comps
 * have valid (non-empty optional) values at the current position. The iterator
 * automatically skips positions where any Comp lacks a value.
 *
 * @note The iterator uses const iterators (Cit) for const Comps and
 * regular iterators (It) for non-const Comps.
 * @note Iteration stops at the size of the smallest Comp to prevent
 * out-of-bounds access.
 *
 * @code
 * SparseArray<int> arr1;
 * SparseArray<float> arr2;
 * Zipper zipper(arr1, arr2);
 * for (auto [val1, val2] : zipper) {
 *     // Process val1 and val2 together
 * }
 * @endcode
 */
template<class... Comps>
class ZipperIterator
{
public:
  friend class ZipperIndexIterator<Comps...>;

  /**
   * @typedef Iterator
   * @brief Selects appropriate iterator type based on Comp constness.
   * @tparam Comp The Comp type to get an iterator for.
   *
   * For const Comps, uses Comp::Cit (const iterator).
   * For non-const Comps, uses Comp::It (regular iterator).
   */
  template<class Comp>
  using Iterator = std::conditional_t<std::is_const_v<Comp>,
                                      typename SparseArray<Comp>::Cit,
                                      typename SparseArray<Comp>::It>;

  /**
   * @typedef ItRef
   * @brief Reference type returned by the iterator.
   * @tparam Comp The Comp type.
   */
  template<class Comp>
  using ItRef = typename Iterator<Comp>::reference;

  /**
   * @typedef Value
   * @brief Selects appropriate value reference type based on Comp
   * constness.
   * @tparam Comp The Comp type.
   *
   * For const Comps, uses Comp::TrueCref (const reference to value).
   * For non-const Comps, uses Comp::TrueRef (reference to value).
   */
  template<class Comp>
  using Value = std::conditional_t<std::is_const_v<Comp>,
                                   typename SparseArray<Comp>::TrueCref,
                                   typename SparseArray<Comp>::TrueRef>;

  /** @brief Tuple type containing values from all Comps. */
  using ValueType = std::tuple<Value<Comps>...>;

  /** @brief Reference type is the same as ValueType for this iterator. */
  using Reference = ValueType;

  /** @brief Pointer type is void as this iterator returns temporary tuples. */
  using Pointer = void;

  /** @brief Difference type for iterator arithmetic. */
  using DifferenceType = std::size_t;

  /** @brief Tuple type containing iterators for all Comps. */
  using IteratorTuple = std::tuple<Iterator<Comps>...>;

  /** @brief Alias for IteratorTuple for backward compatibility. */
  using iterator_tuple = IteratorTuple;

  /** @brief Compile-time index sequence for parameter pack expansion. */
  static constexpr std::index_sequence_for<Comps...> seq {};

  /**
   * @brief Constructs a zipper iterator from a tuple of Comp iterators.
   * @param it_tuple Tuple containing iterators for each Comp.
   * @param scene_array Scene component sparse array
   * @param active_scenes Set of currently active scene names
   * @param scene_states Map of scene names to their state levels
   * @param max Maximum number of elements to iterate over (size of smallest
   * Comp).
   * @param index Starting index position (default: 0).
   * @param min_scene_level Minimum scene state level to include (default:
   * ACTIVE)
   *
   * The constructor advances to the first position where all Comps have
   * valid values. Positions with empty optionals in any Comp are
   * automatically skipped.
   */
  ZipperIterator(
      IteratorTuple const& it_tuple,
      SparseArray<Scene>& scene_array,
      std::unordered_set<std::string> const& active_scenes,
      std::unordered_map<std::string, SceneState> const& scene_states,
      std::size_t max,
      std::size_t index = 0,
      SceneState min_scene_level = SceneState::ACTIVE)
      : _current(it_tuple)
      , _max(max)
      , _scene(scene_array.begin())
      , _scene_size(scene_array.size())
      , _active_scenes(active_scenes)
      , _scene_states(scene_states)
      , _min_scene_level(min_scene_level)
      , _idx(index)
  {
    while (this->_idx < this->_max && !this->all_set(seq)) {
      this->incr_all(seq);
    }
  }

  /**
   * @brief Copy constructor.
   * @param z The zipper iterator to copy from.
   */
  ZipperIterator(ZipperIterator const& z)
      : _current(z._current)
      , _max(z._max)
      , _scene(z._scene)
      , _scene_size(z._scene_size)
      , _active_scenes(z._active_scenes)
      , _scene_states(z._scene_states)
      , _min_scene_level(z._min_scene_level)
      , _idx(z._idx)
  {
  }

  /**
   * @brief Pre-increment operator. Advances to the next valid position.
   * @return Reference to this iterator after incrementing.
   *
   * Advances all underlying iterators and skips positions where any Comp
   * has an empty optional value. Continues until finding a position where all
   * Comps have valid values or reaching the end.
   */
  ZipperIterator& operator++()
  {
    do {
      this->incr_all(seq);
    } while (this->_idx < this->_max && !this->all_set(seq));
    return *this;
  }

  /**
   * @brief Post-increment operator. Advances to the next valid position.
   * @return Copy of the iterator before incrementing.
   */
  ZipperIterator operator++(int)
  {
    auto& tmp = *this;
    ++(*this);
    return tmp;
  }

  /**
   * @brief Dereference operator. Returns tuple of values from all Comps.
   * @return Tuple containing references to values from each Comp at the
   * current position.
   */
  ValueType operator*() { return this->to_value(seq); }

  /**
   * @brief Arrow operator. Returns tuple of values from all Comps.
   * @return Same as operator*, since we return tuples by value.
   */
  ValueType operator->() { return *(*this); }

  /**
   * @brief Equality comparison operator.
   * @param rhs The iterator to compare with.
   * @return true if both iterators are at the same index position, false
   * otherwise.
   */
  bool operator==(ZipperIterator const& rhs) { return this->_idx == rhs._idx; }

  /**
   * @brief Inequality comparison operator.
   * @param rhs The iterator to compare with.
   * @return true if iterators are at different index positions, false
   * otherwise.
   */
  bool operator!=(ZipperIterator const& rhs) { return !(*this == rhs); }

private:
  /**
   * @brief Increments all underlying iterators simultaneously.
   * @tparam Is Index sequence for parameter pack expansion.
   *
   * Increments the internal index and advances each iterator in the tuple.
   */
  template<std::size_t... Is>
  void incr_all(std::index_sequence<Is...> /*unused*/)
  {
    if (this->_idx < this->_scene_size) {
      this->_scene++;
    }
    this->_idx += 1;
    (std::get<Is>(this->_current)++, ...);
  }

  /**
   * @brief Checks if all Comps have valid values at the current position.
   * @tparam Is Index sequence for parameter pack expansion.
   * @return true if all iterators point to valid (non-empty optional) values
   * and the entity's scene meets the minimum level requirement, false
   * otherwise.
   */
  template<std::size_t... Is>
  bool all_set(std::index_sequence<Is...> /*unused*/) const
  {
    // Check if entity is in a scene with sufficient level
    // Only check scene if we're within the scene array bounds
    if (this->_idx < this->_scene_size) {
      // Ensure the scene iterator points to a valid optional before
      // dereferencing
      if (this->_scene->has_value()) {
        const auto& scene_name = this->_scene->value().scene_name;

        // Look up the scene state
        auto it = this->_scene_states.find(scene_name);
        if (it != this->_scene_states.end()) {
          // Check if scene state meets minimum level requirement
          // Using the numeric values: DISABLED=0, ACTIVE=1, MAIN=2
          if (static_cast<std::uint8_t>(it->second)
              >= static_cast<std::uint8_t>(this->_min_scene_level))
          {
            // Scene meets minimum level, check if all components are set
            return ((*(std::get<Is>(this->_current))).has_value() && ...);
          }
          // Scene level is too low, skip this entity
          return false;
        }

        // Scene not found in states map, skip this entity
        return false;
      }
    }

    // Entity has no scene component, include it by default
    return ((*(std::get<Is>(this->_current))).has_value() && ...);
  }

  /**
   * @brief Extracts values from all iterators and returns them as a tuple.
   * @tparam Is Index sequence for parameter pack expansion.
   * @return Tuple containing references to the actual values (unwrapped from
   * optionals).
   */
  template<std::size_t... Is>
  ValueType to_value(std::index_sequence<Is...> /*unused*/)
  {
    return std::forward_as_tuple(std::get<Is>(this->_current)->value()...);
  }

  IteratorTuple
      _current;  ///< Tuple of current iterator positions for each Comp.
  std::size_t _max;  ///< Maximum iteration count (size of smallest Comp).
  Iterator<Scene> _scene;
  std::size_t _scene_size;
  std::unordered_set<std::string> const& _active_scenes;
  std::unordered_map<std::string, SceneState> const& _scene_states;
  SceneState _min_scene_level;  ///< Minimum scene state level to include

protected:
  std::size_t _idx;  ///< Current index position in the iteration.
};

/**
 * @class Zipper
 * @brief Range adapter that simultaneously iterates over multiple Comps.
 * @tparam Comps Variadic template parameter pack of Comp types to zip
 * together.
 *
 * This class provides a convenient interface for parallel iteration over
 * multiple Comps. It returns a ZipperIterator that yields tuples of values
 * from each Comp where all Comps have valid (non-empty optional)
 * values at the same position.
 *
 * @note The zipper respects the size of the smallest Comp to prevent
 * out-of-bounds access.
 * @note Only positions where all Comps have valid values are yielded
 * during iteration.
 *
 * @code
 * SparseArray<Position> positions;
 * SparseArray<Velocity> velocities;
 *
 * for (auto [pos, vel] : Zipper(positions, velocities)) {
 *     pos.x += vel.dx;
 *     pos.y += vel.dy;
 * }
 * @endcode
 */
template<class... Comps>
class Zipper
{
public:
  /** @brief Iterator type for this zipper. */
  using Iterator = ZipperIterator<Comps...>;

  /** @brief Tuple type containing iterators for all Comps. */
  using IteratorTuple = typename Iterator::iterator_tuple;

  /**
   * @brief Constructs a zipper from multiple Comp references.
   * @param r Registry reference containing all components and scene data
   * @param min_scene_level Minimum scene state level to include (default:
   * ACTIVE)
   *
   * Computes the minimum size among all Comps and creates begin/end
   * iterator tuples. The zipper will iterate up to the size of the smallest
   * Comp.
   *
   * The min_scene_level parameter controls which scenes are included:
   * - SceneState::DISABLED: Include all scenes (no filtering)
   * - SceneState::ACTIVE: Include ACTIVE and MAIN scenes (default)
   * - SceneState::MAIN: Include only MAIN scenes
   */
  Zipper(Registry& r, SceneState min_scene_level = SceneState::ACTIVE)
      : _size(compute_size(r))
      , _begin(std::make_tuple(r.get_components<Comps>().begin()...))
      , _end(compute_end(r))
      , _scenes(r.get_components<Scene>())
      , _active_scenes(r.get_active_scenes_set())
      , _scene_states(r.get_scene_states())
      , _min_scene_level(min_scene_level)
  {
  }

  /**
   * @brief Returns an iterator to the beginning of the zipped range.
   * @return ZipperIterator positioned at the first valid element (where all
   * Comps have values).
   */
  Iterator begin()
  {
    return ZipperIterator<Comps...>(this->_begin,
                                    this->_scenes,
                                    this->_active_scenes,
                                    this->_scene_states,
                                    this->_size,
                                    0,
                                    this->_min_scene_level);
  }

  /**
   * @brief Returns an iterator to the end of the zipped range.
   * @return ZipperIterator positioned one past the last valid element.
   */
  Iterator end()
  {
    return ZipperIterator<Comps...>(this->_end,
                                    this->_scenes,
                                    this->_active_scenes,
                                    this->_scene_states,
                                    this->_size,
                                    this->_size,
                                    this->_min_scene_level);
  }

private:
  /**
   * @brief Computes the minimum size among all provided Comps.
   * @param Comps Variadic pack of Comp references.
   * @return Size of the smallest Comp.
   */
  static std::size_t compute_size(Registry const& r)
  {
    return std::min({r.get_components<Comps>().size()...});
  }

  /**
   * @brief Computes the end iterator tuple for all Comps.
   * @param Comps Variadic pack of Comp references.
   * @return Tuple of iterators, each positioned at begin() + min_size for its
   * Comp.
   */
  IteratorTuple compute_end(Registry& r)
  {
    return std::make_tuple(r.get_components<Comps>().begin() + _size...);
  }

  std::size_t _size;  ///< Size of the smallest Comp (iteration limit).
  IteratorTuple _begin;  ///< Tuple of begin iterators for all Comps.
  IteratorTuple _end;  ///< Tuple of end iterators for all Comps.
  SparseArray<Scene>& _scenes;
  std::unordered_set<std::string> const& _active_scenes;
  std::unordered_map<std::string, SceneState> const& _scene_states;
  SceneState _min_scene_level;  ///< Minimum scene state level to include
};
