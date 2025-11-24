#pragma once

#include <optional>
#include <stdexcept>
#include <vector>

template<typename Component>
class SparseArray : public std::vector<std::optional<Component>>
{
public:
  using value = std::optional<Component>;
  using vtype = std::vector<value>;
  using ref = value&;
  using cref = value const&;
  using true_ref = Component&;
  using true_cref = Component const&;
  using size_type = typename vtype::size_type;
  using it = typename vtype::iterator;
  using cit = typename vtype::const_iterator;

  void reserve_init(size_type pos)
  {
    this->reserve(pos + 1);
    while ((pos + 1) > this->size()) {
      this->push_back({});
    }
  }

  ref insert_at(size_type pos, Component&& v)
  {
    this->reserve_init(pos);
    return *this->emplace(this->begin() + pos, v);
  }

  template<typename... Params>
  ref insert_at(size_type pos, Params&&... params)
  {
    this->reserve_init(pos);
    return *this->emplace(this->begin() + pos, Component(params...));
  }

  ref insert_at(size_type pos, Component const& v)
  {
    this->reserve_init(pos);
    return *this->emplace(this->begin() + pos, v);
  }

  void erase(size_type pos)
  {
    if (pos < this->size()) {
      (*this)[pos].reset();
    }
  }

  size_type get_index(value const& val) const
  {
    for (size_type i = 0; i < this->size(); ++i) {
      if ((*this)[i] == val) {
        return i;
      }
    }
    throw std::out_of_range("no matching value");
  }
};
