#pragma once

#include <unordered_map>
#include <utility>

template<typename A, typename B>
class TwoWayMap
{
public:
  TwoWayMap() = default;

  TwoWayMap(std::initializer_list<std::pair<A, B>> const& list)
  {
    for (auto const& i : list) {
      this->insert(i.first, i.second);
    }
  };

  void insert(A const& a, B const& b)
  {
    this->_right.insert_or_assign(a, b);
    this->_left.insert_or_assign(b, a);
  }

  B const& at_first(A const& a) const { return _right.at(a); }

  B& at_first(A const& a) { return _right.at(a); }

  A const& at_second(B const& b) const { return _left.at(b); }

  A& at_second(B const& b) { return _left.at(b); }

  bool contains_first(A const& a) { return _right.contains(a); }

  bool contains_second(B const& a) { return _left.contains(a); }

  std::unordered_map<A, B> const& get_first() { return _right; }

  std::unordered_map<B, A> const& get_second() { return _left; }

private:
  std::unordered_map<A, B> _right;
  std::unordered_map<B, A> _left;
};
