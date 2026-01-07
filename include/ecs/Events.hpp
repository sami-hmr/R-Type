#pragma once

#include <functional>

/**
 * @file Events.hpp
 * @brief event wrapper for ECS functionality
 */

/**
 * @class event
 * @brief Encapsulates a event function with priority-based execution ordering
 *
 * In the ECS architecture, events are functions that operate on entities with
 * specific component sets. This class wraps such functions and provides
 * priority ordering to control execution sequence during
 * Registry::run_events().
 *
 * events are stored sorted by priority (higher values execute first), allowing
 * fine-grained control over update order. For example, input events might run
 * before physics, which runs before rendering.
 *
 * @tparam Args Argument types passed to the event function (typically
 * Registry& and component arrays)
 *
 * @note events are compared using operator< where higher priority values sort
 * first
 * @note Priority defaults to 0 if not specified
 *
 * @see Registry::add_event()
 * @see Registry::run_events()
 *
 * @code
 * // High priority event (runs first)
 * registry.add_event<Position, Velocity>(
 *     [](Registry& r, auto& positions, auto& velocities) {
 *         // Update positions based on velocities
 *     },
 *     100 // High priority
 * );
 *
 * // Lower priority event (runs after)
 * registry.add_event<Position, Sprite>(
 *     [](Registry& r, auto& positions, auto& sprites) {
 *         // Update sprite positions for rendering
 *     },
 *     50 // Lower priority
 * );
 * @endcode
 */
template<typename EventType>
class Event
{
public:
  /**
   * @brief Constructs a event with a function and priority
   * @param fn The event function to execute
   * @param priority Execution priority (higher values run first, default: 0)
   */
  explicit Event(std::function<void(EventType const &)> fn, std::size_t priority)
      : _priority(priority)
      , _fn(std::move(fn))
  {
  }

  /**
   * @brief Executes the event function with provided arguments
   * @param args Arguments to forward to the event function
   */
  void operator()(EventType const &event) const
  {
    this->_fn(event);
  }

  /**
   * @brief Compares events by priority for sorting
   * @param other event to compare against
   * @return true if this event has higher priority (should run first)
   *
   * @note Used by std::upper_bound in Registry::add_event() to maintain
   *       sorted order
   */
  bool operator<(Event const& other) const
  {
    return this->_priority > other._priority;
  }

private:
  size_t _priority;  ///< Execution priority (public for sorting access)
  std::function<void(EventType const &)> _fn;  ///< The wrapped event function
};
