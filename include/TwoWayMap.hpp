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

  B const& at(A const& a) const { return _right.at(a); }

  B& at(A const& a) { return _right.at(a); }

  A const& at(B const& b) const { return _left.at(b); }

  A& at(B const& b) { return _left.at(b); }

private:
  std::unordered_map<A, B> _right;
  std::unordered_map<B, A> _left;
};
