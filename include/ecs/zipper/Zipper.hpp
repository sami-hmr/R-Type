#pragma once

#include <algorithm>
#include <tuple>
#include <type_traits>

template<class... Containers>
class ZipperIndexIterator;

template<class... Containers>
class ZipperIterator
{
public:
  friend class ZipperIndexIterator<Containers...>;

  template<class Container>
  using iterator = std::conditional_t<std::is_const_v<Container>,
                                      typename Container::cit,
                                      typename Container::it>;

  template<class Container>
  using it_ref = typename iterator<Container>::reference;

  template<class Container>
  using value = std::conditional_t<std::is_const_v<Container>,
                                   typename Container::true_cref,
                                   typename Container::true_ref>;

  using value_type = std::tuple<value<Containers>...>;

  using reference = value_type;
  using pointer = void;
  using difference_type = std::size_t;

  using iterator_tuple = std::tuple<iterator<Containers>...>;
  static constexpr std::index_sequence_for<Containers...> _seq {};

  ZipperIterator(iterator_tuple const& it_tuple,
                 std::size_t max,
                 std::size_t index = 0)
      : current_(it_tuple)
      , max_(max)
      , idx_(index)
  {
    while (this->idx_ < this->max_ && !this->all_set(_seq)) {
      this->incr_all(_seq);
    }
  }

  ZipperIterator(ZipperIterator const& z)
      : current_(z.current_)
      , max_(z.max_)
      , idx_(z.idx_)
  {
  }

  ZipperIterator& operator++()
  {
    do {
      this->incr_all(_seq);
    } while (this->idx_ < this->max_ && !this->all_set(_seq));
    return *this;
  }

  ZipperIterator operator++(int)
  {
    auto& tmp = *this;
    ++(*this);
    return tmp;
  }

  value_type operator*() { return this->to_value(_seq); }

  value_type operator->() { return *(*this); }

  bool operator==(ZipperIterator const& rhs) { return this->idx_ == rhs.idx_; }

  bool operator!=(ZipperIterator const& rhs) { return !(*this == rhs); }

private:
  template<std::size_t... Is>
  void incr_all(std::index_sequence<Is...>)
  {
    this->idx_ += 1;
    (std::get<Is>(this->current_)++, ...);
  }

  template<std::size_t... Is>
  bool all_set(std::index_sequence<Is...>)
  {
    return ((*(std::get<Is>(this->current_))).has_value() && ...);
  }

  template<std::size_t... Is>
  value_type to_value(std::index_sequence<Is...>)
  {
    return std::forward_as_tuple(std::get<Is>(this->current_)->value()...);
  }

  iterator_tuple current_;
  std::size_t max_;

protected:
  std::size_t idx_;
};

template<class... Containers>
class Zipper
{
public:
  using iterator = ZipperIterator<Containers...>;
  using iterator_tuple = typename iterator::iterator_tuple;

  Zipper(Containers&... cs)
      : _size(compute_size_(cs...))
      , _begin(std::make_tuple(cs.begin()...))
      , _end(compute_end_(cs...))
  {
  }

  iterator begin()
  {
    return ZipperIterator<Containers...>(this->_begin, this->_size);
  }

  iterator end()
  {
    return ZipperIterator<Containers...>(this->_end, this->_size, this->_size);
  }

private:
  static std::size_t compute_size_(Containers&... containers)
  {
    return std::min({containers.size()...});
  }

  static iterator_tuple compute_end_(Containers&... containers)
  {
    return std::make_tuple(containers.begin()
                           + compute_size_(containers...)...);
  }

private:
  std::size_t _size;
  iterator_tuple _begin;
  iterator_tuple _end;
};
