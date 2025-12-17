
#include "Json/JsonParser.hpp"
#include "NetworkShared.hpp"
#include "ecs/Registry.hpp"
#include "plugin/events/LoggerEvent.hpp"

/**
 * @file EmitEvent.hpp
 * @brief Event emission helper functions with automatic network synchronization
 *
 * These functions wrap Registry::emit() to automatically broadcast events
 * to network clients via EventBuilder events. This ensures events are
 * synchronized across the network in multiplayer scenarios.
 */

/**
 * @brief Emits an event constructed from JSON parameters
 *
 * This function constructs an event from JSON configuration and emits it
 * both locally and to the network. It handles unknown events gracefully
 * by logging an error instead of crashing.
 *
 * @param r Registry instance to emit the event through
 * @param id Event type identifier (must be registered via Registry::on())
 * @param params JSON object containing event parameters
 *
 * @note This function emits two events:
 *       1. EventBuilder (for network synchronization)
 *       2. The actual event (for local handlers)
 * @note If event ID is unknown, emits a LogEvent with ERROR level
 *
 * @see Registry::emit()
 * @see EventBuilder
 */
inline void emit_event(Registry& r,
                       std::string const& id,
                       JsonObject const& params)
{
  try {
    r.emit<EventBuilder>(EventBuilder(id, r.get_event_with_id(id, params)));
  } catch (std::out_of_range const&) {
    r.emit<LogEvent>("init", LogLevel::ERROR, "unknow event");
  }
  r.emit(id, params);
}

/**
 * @brief Emits a pre-constructed event with network synchronization
 *
 * This templated function emits an already-constructed event object,
 * serializing it for network transmission via EventBuilder before
 * triggering local event handlers.
 *
 * @tparam Event Event type (must satisfy the event concept)
 * @param r Registry instance to emit the event through
 * @param id Event type identifier string
 * @param event The event object to emit
 *
 * @note Event type must satisfy the event concept (bytable + entity_convertible
 * + json_buildable)
 * @note Emits EventBuilder first for network, then the actual event locally
 * @note Logs error if event type is not registered
 *
 * @see Registry::emit()
 * @see EventBuilder
 */
template<event Event>
void emit_event(Registry& r, std::string const& id, Event event)
{
  try {
    r.emit<EventBuilder>(EventBuilder(id, event.to_bytes()));
  } catch (std::out_of_range const&) {
    r.emit<LogEvent>("init", LogLevel::ERROR, "unknown event");
  }
  r.emit(id, event.to_bytes());
}
