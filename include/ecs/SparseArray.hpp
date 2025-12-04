#pragma once

#include <optional>
#include <stdexcept>
#include <vector>

/**
 * @brief A sparse array implementation that stores components in an optional vector.
 *
 * @tparam Component The type of component to be stored in the sparse array.
 */
template<typename Component>
class SparseArray : public std::vector<std::optional<Component>>
{
public:
  using Value = std::optional<Component>; /**< Type alias for the optional component value stored in the sparse array. */
  using Vtype = std::vector<Value>; /**< Type alias for the underlying vector type. */
  using Ref = Value&; /**< Type alias for a reference to the optional component value. */
  using Cref = Value const&; /**< Type alias for a const reference to the optional component value. */
  using TrueRef = Component&; /**< Type alias for a reference to the actual component value. */
  using TrueCref = Component const&; /**< Type alias for a const reference to the actual component value. */
  using SizeType = typename Vtype::size_type; /**< Type alias for the size type of the underlying vector. */
  using It = typename Vtype::iterator; /**< Type alias for an iterator to the underlying vector. */
  using Cit = typename Vtype::const_iterator; /**< Type alias for a const iterator to the underlying vector. */

  /**
   * @brief Ensures the underlying vector has enough capacity and size to accommodate the given position.
   *
   * @param pos The position to reserve space for.
   */
  void reserve_init(SizeType pos)
  {
    this->reserve(pos + 1);
    while ((pos + 1) > this->size()) {
      this->push_back({});
    }
  }

  /**
   * @brief   Inserts a component at the specified position in the sparse array.
   *
   * @param pos The position to insert the component at.
   * @param v The component to insert.
   * @return Ref A reference to the inserted component.
   */
  Ref insert_at(SizeType pos, Component&& v)
  {
    this->reserve_init(pos);
    return *this->emplace(this->begin() + pos, v);
  }

  /**
   * @brief   Inserts a component at the specified position in the sparse array.
   *
   * @tparam Params The types of the parameters to construct the component.
   * @param pos The position to insert the component at.
   * @param params The parameters to construct the component.
   * @return Ref A reference to the inserted component.
   */
  template<typename... Params>
  Ref insert_at(SizeType pos, Params&&... params)
  {
    this->reserve_init(pos);
    return *this->emplace(this->begin() + pos, Component(params...));
  }

  /**
   * @brief   Inserts a component at the specified position in the sparse array.
   *
   * @param pos The position to insert the component at.
   * @param v The component to insert.
   * @return Ref A reference to the inserted component.
   */
  Ref insert_at(SizeType pos, Component const& v)
  {
    this->reserve_init(pos);
    return *this->emplace(this->begin() + pos, v);
  }

  /**
   * @brief  Erases the component at the specified position in the sparse array.
   *
   * @param pos The position of the component to erase.
   */
  void erase(SizeType pos)
  {
    if (pos < this->size() && (*this)[pos].has_value()) {
      (*this)[pos].reset();
    }
  }

  /**
   * @brief   Finds the index of the specified component value in the sparse array.
   *
   * @param val The component value to find.
   * @return SizeType The index of the component value.
   * @throws std::out_of_range if the component value is not found.
   */
  SizeType get_index(Value const& val) const
  {
    for (SizeType i = 0; i < this->size(); ++i) {
      if ((*this)[i] == val) {
        return i;
      }
    }
    throw std::out_of_range("no matching value");
  }
};
