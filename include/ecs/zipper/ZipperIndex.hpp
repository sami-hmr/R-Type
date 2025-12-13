#pragma once

#include <algorithm>
#include <cstddef>
#include <tuple>

#include "Zipper.hpp"
#include "ecs/Registry.hpp"

/**
 * @class ZipperIndexIterator
 * @brief Iterator that zips multiple Comps and includes the current index
 * in the result.
 * @tparam Comps Variadic template parameter pack of container types to
 * iterate over.
 *
 * This iterator extends ZipperIterator by prepending the current iteration
 * index to the tuple of values. It allows tracking both the position in the
 * iteration and the actual values from each container.
 *
 * @note The index represents the logical position in the iteration, which may
 * differ from the actual container indices due to skipped empty optional
 * values.
 * @note Uses ZipperIterator as a base for the core iteration logic.
 *
 * @example
 * @code
 * SparseArray<int> arr1;
 * SparseArray<float> arr2;
 * ZipperIndex zipper(arr1, arr2);
 * for (auto [index, val1, val2] : zipper) {
 *     std::cout << "Position " << index << ": " << val1 << ", " << val2 <<
 * std::endl;
 * }
 * @endcode
 */
template<class... Comps>
class ZipperIndexIterator
{
public:
  /** @brief Base iterator type (ZipperIterator) used internally. */
  using Base = ZipperIterator<Comps...>;

  /**
   * @typedef Value
   * @brief Value type extracted from each container through the base iterator.
   * @tparam Container The container type.
   */
  template<class Container>
  using Value = typename Base::template Value<Container>;

  /** @brief Tuple type containing the index followed by values from all
   * Comps. */
  using ValueType = std::tuple<std::size_t, Value<Comps>...>;

  /**
   * @brief Constructs a zipper index iterator by forwarding arguments to the
   * base iterator.
   * @tparam Args Argument types to forward to ZipperIterator constructor.
   * @param args Arguments forwarded to construct the underlying ZipperIterator.
   */
  template<class... Args>
  ZipperIndexIterator(Args&&... args)
      : _base(args...)
  {
  }

  /**
   * @brief Pre-increment operator. Advances to the next valid position.
   * @return Reference to this iterator after incrementing.
   *
   * Delegates to the underlying ZipperIterator's increment operator, which
   * automatically skips positions where any container has empty optional
   * values.
   */
  ZipperIndexIterator& operator++()
  {
    ++this->_base;
    return *this;
  }

  /**
   * @brief Post-increment operator. Advances to the next valid position.
   * @return Copy of the iterator before incrementing.
   */
  ZipperIndexIterator operator++(int)
  {
    auto& tmp = *this;
    ++(*this);
    return tmp;
  }

  /**
   * @brief Dereference operator. Returns tuple with index and values from all
   * Comps.
   * @return Tuple where the first element is the current index, followed by
   * values from each container.
   */
  ValueType operator*()
  {
    return std::tuple_cat(std::make_tuple(this->_base._idx), *this->_base);
  }

  /**
   * @brief Arrow operator. Returns tuple with index and values from all
   * Comps.
   * @return Same as operator*, since we return tuples by value.
   */
  ValueType operator->() { return *(*this); }

  /**
   * @brief Equality comparison operator.
   * @param rhs The iterator to compare with.
   * @return true if both iterators are at the same index position, false
   * otherwise.
   */
  bool operator==(ZipperIndexIterator const& rhs)
  {
    return this->_base._idx == rhs._base._idx;
  }

  /**
   * @brief Inequality comparison operator.
   * @param rhs The iterator to compare with.
   * @return true if iterators are at different index positions, false
   * otherwise.
   */
  bool operator!=(ZipperIndexIterator const& rhs) { return !(*this == rhs); }

private:
  Base _base;  ///< Underlying ZipperIterator that provides the core iteration
               ///< logic.
};

/**
 * @class ZipperIndex
 * @brief Range adapter that zips multiple Comps and includes indices in
 * the iteration.
 * @tparam Comps Variadic template parameter pack of container types to zip
 * together.
 *
 * This class extends the functionality of Zipper by including the current
 * iteration index in each yielded tuple. It's useful when you need to know the
 * position during iteration, for example when processing entities by their
 * index or performing indexed operations.
 *
 * @note The index is the logical iteration position, not necessarily the
 * container index, since positions with empty optionals are skipped.
 * @note Like Zipper, it respects the size of the smallest container.
 *
 * @example
 * @code
 * SparseArray<Position> positions;
 * SparseArray<Velocity> velocities;
 *
 * for (auto [idx, pos, vel] : ZipperIndex(positions, velocities)) {
 *     std::cout << "Entity " << idx << " at (" << pos.x << "," << pos.y << ")"
 * << std::endl; pos.x += vel.dx; pos.y += vel.dy;
 * }
 * @endcode
 */
template<class... Comps>
class ZipperIndex
{
public:
  /** @brief Iterator type for this indexed zipper. */
  using Iterator = ZipperIndexIterator<Comps...>;

  /** @brief Tuple type containing iterators for all Comps. */
  using IteratorTuple = typename Iterator::Base::iterator_tuple;

  /**
   * @brief Constructs an indexed zipper from multiple container references.
   * @param cs Variadic pack of container references to zip together.
   *
   * Computes the minimum size among all Comps and creates begin/end
   * iterator tuples. The zipper will iterate up to the size of the smallest
   * container, yielding tuples that include the iteration index.
   */
  ZipperIndex(Registry& r)
      : _size(compute_size(r))
      , _begin(std::make_tuple(r.get_components<Comps>().begin()...))
      , _end(compute_end(r))
      , _scenes(r.get_components<Scene>())
      , _active_scenes(r.get_current_scene())
  {
  }

  /**
   * @brief Returns an iterator to the beginning of the indexed zipped range.
   * @return ZipperIndexIterator positioned at the first valid element.
   *
   * The returned iterator will yield tuples of (index, value1, value2, ...)
   * where index starts at 0 and increments for each valid position.
   */
  Iterator begin()
  {
    return ZipperIndexIterator<Comps...>(
        this->_begin, this->_scenes, this->_active_scenes, this->_size);
  }

  /**
   * @brief Returns an iterator to the end of the indexed zipped range.
   * @return ZipperIndexIterator positioned one past the last valid element.
   */
  Iterator end()
  {
    return ZipperIndexIterator<Comps...>(this->_end,
                                         this->_scenes,
                                         this->_active_scenes,
                                         this->_size,
                                         this->_size);
  }

private:
  /**
   * @brief Computes the minimum size among all provided Comps.
   * @param Comps Variadic pack of container references.
   * @return Size of the smallest container.
   */
  static std::size_t compute_size(Registry const& r)
  {
    return std::min({r.get_components<Comps>().size()...});
  }

  /**
   * @brief Computes the end iterator tuple for all Comps.
   * @param Comps Variadic pack of container references.
   * @return Tuple of iterators, each positioned at begin() + min_size for its
   * container.
   */
  IteratorTuple compute_end(Registry& r)
  {
    return std::make_tuple(r.get_components<Comps>().begin() + _size...);
  }

  std::size_t _size;  ///< Size of the smallest Comp (iteration limit).
  IteratorTuple _begin;  ///< Tuple of begin iterators for all Comps.
  IteratorTuple _end;  ///< Tuple of end iterators for all Comps.
  SparseArray<Scene>& _scenes;
  std::vector<std::string> _active_scenes;
};
