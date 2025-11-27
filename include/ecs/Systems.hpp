#pragma once

#include <functional>
#include <iostream>

/**
 * @brief Wraps a function with a priority for ordered system execution.
 *
 * @tparam Args The argument types the system function accepts.
 */
template<typename... Args>
class System
{
public:
  /**
   * @brief Constructs a system with the given function and priority.
   *
   * @param fn The function to execute when the system runs.
   * @param priority Execution priority, higher values run first.
   */
  explicit System(std::function<void(Args...)> fn, std::size_t priority)
      : _priority(priority)
      , _fn(std::move(fn))
  {
    if (!static_cast<bool>(this->_fn)) {
      std::cerr << "bordel" << std::endl;
    }
  }

  /**
   * @brief Executes the system function with the provided arguments.
   */
  void operator()(Args&&... args) const
  {
    this->_fn(std::forward<Args>(args)...);
  }

  /**
   * @brief Compares systems by priority for sorting.
   */
  bool operator<(System const& other) const
  {
    return this->_priority > other._priority;
  }

private:
  size_t _priority;
  std::function<void(Args...)> _fn;
};
