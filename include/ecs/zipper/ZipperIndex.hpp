#pragma once

#include <algorithm>
#include <cstddef>
#include <tuple>

#include "Zipper.hpp"

template<class... Containers>
class ZipperIndexIterator
{
public:
  using Base = ZipperIterator<Containers...>;
  template<class Container>
  using Value = Base::template value<Container>;
  using ValueType = std::tuple<std::size_t, Value<Containers>...>;

  template<class... Args>
  ZipperIndexIterator(Args&&... args)
      : _base(args...)
  {
  }

  ZipperIndexIterator& operator++()
  {
    ++this->_base;
    return *this;
  }

  ZipperIndexIterator operator++(int)
  {
    auto& tmp = *this;
    ++(*this);
    return tmp;
  }

  ValueType operator*()
  {
    return std::tuple_cat(std::make_tuple(this->_base.idx_), *this->_base);
  }

  ValueType operator->() { return *(*this); }

  bool operator==(ZipperIndexIterator const& rhs)
  {
    return this->_base.idx_ == rhs._base.idx_;
  }

  bool operator!=(ZipperIndexIterator const& rhs) { return !(*this == rhs); }

private:
  Base _base;
};

template<class... Containers>
class ZipperIndex
{
public:
  using Iterator = ZipperIndexIterator<Containers...>;
  using IteratorTuple = typename Iterator::base::iterator_tuple;

  ZipperIndex(Containers&... cs)
      : _size(compute_size(cs...))
      , _begin(std::make_tuple(cs.begin()...))
      , _end(compute_end(cs...))
  {
  }

  Iterator begin()
  {
    return ZipperIndexIterator<Containers...>(this->_begin, this->_size);
  }

  Iterator end()
  {
    return ZipperIndexIterator<Containers...>(
        this->_end, this->_size, this->_size);
  }

private:
  static std::size_t compute_size(Containers&... containers)
  {
    return std::min(containers.size()...);
  }

  static IteratorTuple compute_end(Containers&... containers)
  {
    return std::make_tuple(containers.begin()
                           + compute_size(containers...)...);
  }


  std::size_t _size;
  IteratorTuple _begin;
  IteratorTuple _end;
};
