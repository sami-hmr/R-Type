#pragma once

#include <algorithm>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

template<class... Containers>
class ZipperIndexIterator;

/**
 * @class ZipperIterator
 * @brief Iterator that simultaneously traverses multiple containers and yields tuples of their elements.
 * @tparam Containers Variadic template parameter pack of container types to iterate over.
 *
 * This iterator implements a zipper pattern that combines multiple containers into a single iteration stream.
 * It only yields values where all containers have valid (non-empty optional) values at the current position.
 * The iterator automatically skips positions where any container lacks a value.
 *
 * @note The iterator uses const iterators (Cit) for const containers and regular iterators (It) for non-const containers.
 * @note Iteration stops at the size of the smallest container to prevent out-of-bounds access.
 *
 * @example
 * @code
 * SparseArray<int> arr1;
 * SparseArray<float> arr2;
 * Zipper zipper(arr1, arr2);
 * for (auto [val1, val2] : zipper) {
 *     // Process val1 and val2 together
 * }
 * @endcode
 */
template<class... Containers>
class ZipperIterator
{
public:
  friend class ZipperIndexIterator<Containers...>;

  /**
   * @typedef Iterator
   * @brief Selects appropriate iterator type based on container constness.
   * @tparam Container The container type to get an iterator for.
   *
   * For const containers, uses Container::Cit (const iterator).
   * For non-const containers, uses Container::It (regular iterator).
   */
  template<class Container>
  using Iterator = std::conditional_t<std::is_const_v<Container>,
                                      typename Container::Cit,
                                      typename Container::It>;

  /**
   * @typedef ItRef
   * @brief Reference type returned by the iterator.
   * @tparam Container The container type.
   */
  template<class Container>
  using ItRef = typename Iterator<Container>::reference;

  /**
   * @typedef Value
   * @brief Selects appropriate value reference type based on container constness.
   * @tparam Container The container type.
   *
   * For const containers, uses Container::TrueCref (const reference to value).
   * For non-const containers, uses Container::TrueRef (reference to value).
   */
  template<class Container>
  using Value = std::conditional_t<std::is_const_v<Container>,
                                   typename Container::TrueCref,
                                   typename Container::TrueRef>;

  /** @brief Tuple type containing values from all containers. */
  using ValueType = std::tuple<Value<Containers>...>;

  /** @brief Reference type is the same as ValueType for this iterator. */
  using Reference = ValueType;
  
  /** @brief Pointer type is void as this iterator returns temporary tuples. */
  using Pointer = void;
  
  /** @brief Difference type for iterator arithmetic. */
  using DifferenceType = std::size_t;

  /** @brief Tuple type containing iterators for all containers. */
  using IteratorTuple = std::tuple<Iterator<Containers>...>;
  
  /** @brief Alias for IteratorTuple for backward compatibility. */
  using iterator_tuple = IteratorTuple;
  
  /** @brief Compile-time index sequence for parameter pack expansion. */
  static constexpr std::index_sequence_for<Containers...> seq {};

  /**
   * @brief Constructs a zipper iterator from a tuple of container iterators.
   * @param it_tuple Tuple containing iterators for each container.
   * @param max Maximum number of elements to iterate over (size of smallest container).
   * @param index Starting index position (default: 0).
   *
   * The constructor advances to the first position where all containers have valid values.
   * Positions with empty optionals in any container are automatically skipped.
   */
  ZipperIterator(IteratorTuple const& it_tuple,
                 std::size_t max,
                 std::size_t index = 0)
      : _current(it_tuple)
      , _max(max)
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
      , _idx(z._idx)
  {
  }

  /**
   * @brief Pre-increment operator. Advances to the next valid position.
   * @return Reference to this iterator after incrementing.
   *
   * Advances all underlying iterators and skips positions where any container
   * has an empty optional value. Continues until finding a position where all
   * containers have valid values or reaching the end.
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
   * @brief Dereference operator. Returns tuple of values from all containers.
   * @return Tuple containing references to values from each container at the current position.
   */
  ValueType operator*() { return this->to_value(seq); }

  /**
   * @brief Arrow operator. Returns tuple of values from all containers.
   * @return Same as operator*, since we return tuples by value.
   */
  ValueType operator->() { return *(*this); }

  /**
   * @brief Equality comparison operator.
   * @param rhs The iterator to compare with.
   * @return true if both iterators are at the same index position, false otherwise.
   */
  bool operator==(ZipperIterator const& rhs) { return this->_idx == rhs._idx; }

  /**
   * @brief Inequality comparison operator.
   * @param rhs The iterator to compare with.
   * @return true if iterators are at different index positions, false otherwise.
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
    this->_idx += 1;
    (std::get<Is>(this->_current)++, ...);
  }

  /**
   * @brief Checks if all containers have valid values at the current position.
   * @tparam Is Index sequence for parameter pack expansion.
   * @return true if all iterators point to valid (non-empty optional) values, false otherwise.
   */
  template<std::size_t... Is>
  bool all_set(std::index_sequence<Is...> /*unused*/)
  {
    return ((*(std::get<Is>(this->_current))).has_value() && ...);
  }

  /**
   * @brief Extracts values from all iterators and returns them as a tuple.
   * @tparam Is Index sequence for parameter pack expansion.
   * @return Tuple containing references to the actual values (unwrapped from optionals).
   */
  template<std::size_t... Is>
  ValueType to_value(std::index_sequence<Is...> /*unused*/)
  {
    return std::forward_as_tuple(std::get<Is>(this->_current)->value()...);
  }

  IteratorTuple _current; ///< Tuple of current iterator positions for each container.
  std::size_t _max;       ///< Maximum iteration count (size of smallest container).

protected:
  std::size_t _idx;       ///< Current index position in the iteration.
};

/**
 * @class Zipper
 * @brief Range adapter that simultaneously iterates over multiple containers.
 * @tparam Containers Variadic template parameter pack of container types to zip together.
 *
 * This class provides a convenient interface for parallel iteration over multiple containers.
 * It returns a ZipperIterator that yields tuples of values from each container where all
 * containers have valid (non-empty optional) values at the same position.
 *
 * @note The zipper respects the size of the smallest container to prevent out-of-bounds access.
 * @note Only positions where all containers have valid values are yielded during iteration.
 *
 * @example
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
template<class... Containers>
class Zipper
{
public:
  /** @brief Iterator type for this zipper. */
  using Iterator = ZipperIterator<Containers...>;
  
  /** @brief Tuple type containing iterators for all containers. */
  using IteratorTuple = typename Iterator::iterator_tuple;

  /**
   * @brief Constructs a zipper from multiple container references.
   * @param cs Variadic pack of container references to zip together.
   *
   * Computes the minimum size among all containers and creates begin/end iterator tuples.
   * The zipper will iterate up to the size of the smallest container.
   */
  Zipper(Containers&... cs)
      : _size(compute_size(cs...))
      , _begin(std::make_tuple(cs.begin()...))
      , _end(compute_end(cs...))
  {
  }

  /**
   * @brief Returns an iterator to the beginning of the zipped range.
   * @return ZipperIterator positioned at the first valid element (where all containers have values).
   */
  Iterator begin()
  {
    return ZipperIterator<Containers...>(this->_begin, this->_size);
  }

  /**
   * @brief Returns an iterator to the end of the zipped range.
   * @return ZipperIterator positioned one past the last valid element.
   */
  Iterator end()
  {
    return ZipperIterator<Containers...>(this->_end, this->_size, this->_size);
  }

private:
  /**
   * @brief Computes the minimum size among all provided containers.
   * @param containers Variadic pack of container references.
   * @return Size of the smallest container.
   */
  static std::size_t compute_size(Containers&... containers)
  {
    return std::min({containers.size()...});
  }

  /**
   * @brief Computes the end iterator tuple for all containers.
   * @param containers Variadic pack of container references.
   * @return Tuple of iterators, each positioned at begin() + min_size for its container.
   */
  static IteratorTuple compute_end(Containers&... containers)
  {
    return std::make_tuple(containers.begin() + compute_size(containers...)...);
  }

  std::size_t _size;      ///< Size of the smallest container (iteration limit).
  IteratorTuple _begin;   ///< Tuple of begin iterators for all containers.
  IteratorTuple _end;     ///< Tuple of end iterators for all containers.
};
