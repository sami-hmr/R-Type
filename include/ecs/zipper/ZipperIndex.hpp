#pragma once

#include <algorithm>
#include <tuple>

#include "Zipper.hpp"

template<class... Containers>
class ZipperIndexIterator
{
public:
  using base = ZipperIterator<Containers...>;
  template<class Container>
  using value = base::template value<Container>;
  using value_type = std::tuple<std::size_t, value<Containers>...>;

  template<class... Args>
  ZipperIndexIterator(Args&&... args)
      : base_(args...)
  {
  }

  ZipperIndexIterator& operator++()
  {
    ++this->base_;
    return *this;
  }

  ZipperIndexIterator operator++(int)
  {
    auto& tmp = *this;
    ++(*this);
    return tmp;
  }

  value_type operator*()
  {
    return std::tuple_cat(std::make_tuple(this->base_.idx_), *this->base_);
  }

  value_type operator->() { return *(*this); }

  bool operator==(ZipperIndexIterator const& rhs)
  {
    return this->base_.idx_ == rhs.base_.idx_;
  }

  bool operator!=(ZipperIndexIterator const& rhs) { return !(*this == rhs); }

private:
  base base_;
};

template<class... Containers>
class ZipperIndex
{
public:
  using iterator = ZipperIndexIterator<Containers...>;
  using iterator_tuple = typename iterator::base::iterator_tuple;

  ZipperIndex(Containers&... cs)
      : _size(compute_size_(cs...))
      , _begin(std::make_tuple(cs.begin()...))
      , _end(compute_end_(cs...))
  {
  }

  iterator begin()
  {
    return ZipperIndexIterator<Containers...>(this->_begin, this->_size);
  }

  iterator end()
  {
    return ZipperIndexIterator<Containers...>(
        this->_end, this->_size, this->_size);
  }

private:
  static std::size_t compute_size_(Containers&... containers)
  {
    return std::min(containers.size()...);
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
