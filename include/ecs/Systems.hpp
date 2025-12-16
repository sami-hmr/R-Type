#pragma once

#include <functional>
#include <iostream>

template<typename... Args>
class System
{
public:
  explicit System(std::function<void(Args...)> fn, std::size_t priority)
      : _priority(priority)
      , _fn(std::move(fn))
  {
  }

  void operator()(Args&&... args) const
  {
    this->_fn(std::forward<Args>(args)...);
  }

  bool operator<(System const& other) const
  {
    return this->_priority > other._priority;
  }

  size_t _priority;
private:
  std::function<void(Args...)> _fn;
};
