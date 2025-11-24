#pragma once

#include <algorithm>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

template<class... Containers>
class ZipperIndexIterator;

template<class... Containers>
class ZipperIterator
{
public:
  friend class ZipperIndexIterator<Containers...>;

  template<class Container>
  using Iterator = std::conditional_t<std::is_const_v<Container>,
                                      typename Container::cit,
                                      typename Container::it>;

  template<class Container>
  using ItRef = typename Iterator<Container>::reference;

  template<class Container>
  using Value = std::conditional_t<std::is_const_v<Container>,
                                   typename Container::true_cref,
                                   typename Container::true_ref>;

  using ValueType = std::tuple<Value<Containers>...>;

  using Reference = ValueType;
  using Pointer = void;
  using DifferenceType = std::size_t;

  using IteratorTuple = std::tuple<Iterator<Containers>...>;
  using iterator_tuple = IteratorTuple;
  static constexpr std::index_sequence_for<Containers...> seq {};

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

  ZipperIterator(ZipperIterator const& z)
      : _current(z._current)
      , _max(z._max)
      , _idx(z._idx)
  {
  }

  ZipperIterator& operator++()
  {
    do {
      this->incr_all(seq);
    } while (this->_idx < this->_max && !this->all_set(seq));
    return *this;
  }

  ZipperIterator operator++(int)
  {
    auto& tmp = *this;
    ++(*this);
    return tmp;
  }

  ValueType operator*() { return this->to_value(seq); }

  ValueType operator->() { return *(*this); }

  bool operator==(ZipperIterator const& rhs) { return this->_idx == rhs._idx; }

  bool operator!=(ZipperIterator const& rhs) { return !(*this == rhs); }

private:
  template<std::size_t... Is>
  void incr_all(std::index_sequence<Is...> /*unused*/)
  {
    this->_idx += 1;
    (std::get<Is>(this->_current)++, ...);
  }

  template<std::size_t... Is>
  bool all_set(std::index_sequence<Is...> /*unused*/)
  {
    return ((*(std::get<Is>(this->_current))).has_value() && ...);
  }

  template<std::size_t... Is>
  ValueType to_value(std::index_sequence<Is...> /*unused*/)
  {
    return std::forward_as_tuple(std::get<Is>(this->_current)->value()...);
  }

  IteratorTuple _current;
  std::size_t _max;

protected:
  std::size_t _idx;
};

template<class... Containers>
class Zipper
{
public:
  using Iterator = ZipperIterator<Containers...>;
  using IteratorTuple = typename Iterator::iterator_tuple;

  Zipper(Containers&... cs)
      : _size(compute_size(cs...))
      , _begin(std::make_tuple(cs.begin()...))
      , _end(compute_end(cs...))
  {
  }

  Iterator begin()
  {
    return ZipperIterator<Containers...>(this->_begin, this->_size);
  }

  Iterator end()
  {
    return ZipperIterator<Containers...>(this->_end, this->_size, this->_size);
  }

private:
  static std::size_t compute_size(Containers&... containers)
  {
    return std::min({containers.size()...});
  }

  static IteratorTuple compute_end(Containers&... containers)
  {
    return std::make_tuple(containers.begin() + compute_size(containers...)...);
  }

  std::size_t _size;
  IteratorTuple _begin;
  IteratorTuple _end;
};
