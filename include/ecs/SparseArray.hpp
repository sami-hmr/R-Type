#pragma once

#include <optional>
#include <stdexcept>
#include <vector>

template<typename Component>
class SparseArray : public std::vector<std::optional<Component>>
{
public:
  using Value = std::optional<Component>;
  using Vtype = std::vector<Value>;
  using Ref = Value&;
  using Cref = Value const&;
  using TrueRef = Component&;
  using TrueCref = Component const&;
  using SizeType = typename Vtype::size_type;
  using It = typename Vtype::iterator;
  using Cit = typename Vtype::const_iterator;

  using value = Value;
  using vtype = Vtype;
  using ref = Ref;
  using cref = Cref;
  using true_ref = TrueRef;
  using true_cref = TrueCref;
  using size_type = SizeType;
  using it = It;
  using cit = Cit;

  void reserve_init(SizeType pos)
  {
    this->reserve(pos + 1);
    while ((pos + 1) > this->size()) {
      this->push_back({});
    }
  }

  Ref insert_at(SizeType pos, Component&& v)
  {
    this->reserve_init(pos);
    return *this->emplace(this->begin() + pos, v);
  }

  template<typename... Params>
  Ref insert_at(SizeType pos, Params&&... params)
  {
    this->reserve_init(pos);
    return *this->emplace(this->begin() + pos, Component(params...));
  }

  Ref insert_at(SizeType pos, Component const& v)
  {
    this->reserve_init(pos);
    return *this->emplace(this->begin() + pos, v);
  }

  void erase(SizeType pos)
  {
    if (pos < this->size()) {
      (*this)[pos].reset();
    }
  }

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
