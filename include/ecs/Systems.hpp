#pragma once

#include <functional>
#include <iostream>

/**
 * @file Systems.hpp
 * @brief System wrapper for ECS functionality
 */

/**
 * @class System
 * @brief Encapsulates a system function with priority-based execution ordering
 *
 * In the ECS architecture, systems are functions that operate on entities with
 * specific component sets. This class wraps such functions and provides
 * priority ordering to control execution sequence during
 * Registry::run_systems().
 *
 * Systems are stored sorted by priority (higher values execute first), allowing
 * fine-grained control over update order. For example, input systems might run
 * before physics, which runs before rendering.
 *
 * @tparam Args Argument types passed to the system function (typically
 * Registry& and component arrays)
 *
 * @note Systems are compared using operator< where higher priority values sort
 * first
 * @note Priority defaults to 0 if not specified
 *
 * @see Registry::add_system()
 * @see Registry::run_systems()
 *
 * @code
 * // High priority system (runs first)
 * registry.add_system<Position, Velocity>(
 *     [](Registry& r, auto& positions, auto& velocities) {
 *         // Update positions based on velocities
 *     },
 *     100 // High priority
 * );
 *
 * // Lower priority system (runs after)
 * registry.add_system<Position, Sprite>(
 *     [](Registry& r, auto& positions, auto& sprites) {
 *         // Update sprite positions for rendering
 *     },
 *     50 // Lower priority
 * );
 * @endcode
 */
template<typename... Args>
class System
{
public:
  /**
   * @brief Constructs a system with a function and priority
   * @param fn The system function to execute
   * @param priority Execution priority (higher values run first, default: 0)
   */
  explicit System(std::function<void(Args...)> fn, std::size_t priority)
      : _priority(priority)
      , _fn(std::move(fn))
  {
  }

  /**
   * @brief Executes the system function with provided arguments
   * @param args Arguments to forward to the system function
   */
  void operator()(Args&&... args) const
  {
    this->_fn(std::forward<Args>(args)...);
  }

  /**
   * @brief Compares systems by priority for sorting
   * @param other System to compare against
   * @return true if this system has higher priority (should run first)
   *
   * @note Used by std::upper_bound in Registry::add_system() to maintain
   *       sorted order
   */
  bool operator<(System const& other) const
  {
    return this->_priority > other._priority;
  }

  size_t _priority = 1;  ///< Execution priority (public for sorting access)
private:
  std::function<void(Args...)> _fn;  ///< The wrapped system function
};
