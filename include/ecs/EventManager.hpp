#pragma once

/**
 * @concept event
 * @brief Requires a type to be a valid event with serialization and JSON
 * construction
 *
 * A valid event type must:
 * - Satisfy bytable (binary serialization)
 * - Satisfy entity_convertible (entity ID remapping)
 * - Satisfy json_buildable (construct from JSON for testing/network)
 *
 * @tparam T The type to check
 *
 * @details
 * Events extend component requirements with JSON construction for:
 * - Network event emission from JSON
 * - Test event creation
 * - Dynamic event building
 *
 * @code
 * struct DamageEvent {
 *   Registry::Entity target;
 *   int amount;
 *
 *   // Bytable
 *   DEFAULT_BYTE_CONSTRUCTOR(DamageEvent, ...)
 *   DEFAULT_SERIALIZE(...)
 *
 *   // Entity-convertible
 *   CHANGE_ENTITY(result.target = map.at(target);)
 *
 *   // Json-buildable
 *   DamageEvent(Registry& r, JsonObject const& obj)
 *     : target(get_value_copy<int>(r, obj, "target").value())
 *     , amount(get_value_copy<int>(r, obj, "amount").value())
 *   {}
 * };
 *
 * static_assert(event<DamageEvent>);
 * @endcode
 *
 * @see component concept
 * @see json_buildable in EventConcept.hpp
 */

#include <any>
#include <typeindex>
#include <vector>

#include "Registry.hpp"
#include "ecs/Events.hpp"
#include "plugin/Byte.hpp"
#include "plugin/events/EventConcept.hpp"

template<typename T>
concept event = bytable<T> && entity_convertible<T> && json_buildable<T>;

class EventManager
{
public:
  /**
   * @brief Type alias for event handler identifiers
   *
   * Handler IDs are unique UUIDs generated when registering event handlers.
   * Used to unregister specific handlers via off().
   */

  // ============================================================================
  // EVENT SYSTEM
  // ============================================================================
  //
  // Provides type-safe event handling with multiple emission formats.
  //
  // Events are user-defined types that trigger registered handler functions.
  // The system supports:
  // - Type-safe handlers with automatic parameter binding
  // - JSON-based event emission for network/config integration
  // - Binary serialization for efficient network transmission
  // - Handler registration/unregistration with unique IDs
  //
  // Handler Lifecycle:
  // 1. Register handler with on() - returns unique ID
  // 2. Handlers execute when emit() is called
  // 3. Unregister with off() (specific) or off_all() (all handlers)
  //
  // Emission Formats:
  // - emit(Event{...})           - Direct type-safe emission
  // - emit<Event>(json)          - JSON deserialization
  // - emit<Event>(bytes, length) - Binary deserialization
  //
  // Network Integration:
  // Events can be serialized to JSON or binary format for network transmission,
  // then deserialized and emitted on remote Registry instances.

  /**
   * @brief Register an event handler with unique identifier.
   *
   * Creates a callback that executes when the specified event type is emitted.
   * Returns a unique string ID for later unregistration with off().
   *
   * The handler function receives the event object and Registry reference:
   * @code
   * void handler(const EventType& evt, Registry& reg) { ... }
   * @endcode
   *
   * Multiple handlers can be registered for the same event type - they execute
   * in registration order. Each handler gets a unique ID for selective removal.
   *
   * @tparam Event Event type (must satisfy event_type concept)
   * @param handler Callback function taking (const Event&, Registry&)
   * @return Unique string ID for unregistering this specific handler
   *
   * @note Handler is not invoked immediately - only when emit() is called
   * @note ID must be stored if you need to unregister later
   * @note Handlers persist across scenes unless explicitly removed
   *
   * @throws std::bad_alloc If handler registration fails
   *
   * @code
   * struct PlayerDied {
   *   std::size_t player_id;
   *   std::string cause;
   * };
   *
   * std::string handler_id = registry.on<PlayerDied>(
   *   [](const PlayerDied& evt, Registry& r) {
   *     std::cout << "Player " << evt.player_id
   *               << " died: " << evt.cause << "\n";
   *     // Spawn explosion effect, update score, etc.
   *   }
   * );
   * @endcode
   *
   * @code
   * // Audio system handler
   * auto audio_id = registry.on<EnemyDestroyed>(
   *   [](const EnemyDestroyed& e, Registry& r) {
   *     play_sound("explosion.wav");
   *   }
   * );
   *
   * // Score system handler
   * auto score_id = registry.on<EnemyDestroyed>(
   *   [](const EnemyDestroyed& e, Registry& r) {
   *     add_score(e.points);
   *   }
   * );
   *
   * // VFX system handler
   * auto vfx_id = registry.on<EnemyDestroyed>(
   *   [](const EnemyDestroyed& e, Registry& r) {
   *     spawn_explosion_at(e.position);
   *   }
   * );
   * // All three handlers execute when event is emitted
   * @endcode
   *
   * @code
   * auto id = registry.on<LevelComplete>(
   *   [](const LevelComplete& evt, Registry& r) {
   *     // Modify components
   *     auto& players = r.get_components<Player>();
   *     for (auto& player : players) {
   *       if (player) player->level_up();
   *     }
   *
   *     // Emit new events
   *     r.emit(ShowLevelCompleteUI{evt.score});
   *
   *     // Change scenes
   *     r.set_current_scene("victory_screen");
   *   }
   * );
   * @endcode
   *
   * @see emit() to trigger handlers
   * @see off() to unregister specific handler
   * @see off_all() to remove all handlers for event type
   */
  template<event EventType>
  void on(std::string const& name,
          std::function<void(const EventType&)> handler,
          std::size_t priority = 1)
  {
    std::type_index type_id(typeid(EventType));
    _index_getter.insert(type_id, name);
    if (!_entity_converter.contains(name)) {
      _entity_converter.insert_or_assign(
          name,
          [](ByteArray const& b,
             std::unordered_map<Registry::Entity, Registry::Entity> const& map)
          { return EventType(b).change_entity(map).to_bytes(); });
    }

    if (!_byte_emitter.contains(name)) {
      _byte_emitter.insert_or_assign(
          name,
          [this](ByteArray const& data)
          { return this->emit<EventType>(EventType(data)); });
    }

    this->add_event_builder<EventType>();

    return this->on<EventType>(handler, priority);
  }

  /**
   * @brief Construct an event from JSON and serialize to binary.
   *
   * Builds an event object from JSON parameters using the registered event
   * builder, then serializes it to binary format. The result can be transmitted
   * over the network or stored.
   *
   * The event type must have been registered with add_event_builder() (which
   * happens automatically when using on()).
   *
   * @param id Event type name (string identifier)
   * @param params JSON object with event parameters
   * @return Binary representation of the constructed event
   *
   * @throws std::out_of_range If event type not registered
   * @throws std::exception If JSON is malformed or event construction fails
   *
   * @note Event type must be registered via add_event_builder()
   * @note Returned bytes can be deserialized with Event::from_bytes()
   *
   * @code
   * // Server builds event from JSON
   * JsonObject json = {
   *   {"player_id", 42},
   *   {"position", {{"x", 100.0}, {"y", 200.0}}}
   * };
   *
   * ByteArray event_data = registry.get_event_with_id("PlayerMoved", json);
   *
   * // Send over network
   * send_to_clients(event_data);
   *
   * // Client receives and emits
   * registry.emit<PlayerMoved>(event_data.data(), event_data.size());
   * @endcode
   *
   * @see add_event_builder() to register event types
   * @see emit<Event>(const byte*, std::size_t) to emit binary events
   * @see on() which registers builders automatically
   */
  ByteArray get_event_with_id(Registry& r,
                              std::string const&,
                              JsonObject const&);

  /**
   * @brief Remove all event handlers for a specific event type.
   *
   * Unregisters every handler that was registered for the given event type.
   * After this call, emitting the event will have no effect until new
   * handlers are registered.
   *
   * @tparam Event Event type (must satisfy event_type concept)
   *
   * @note Safe to call even if no handlers are registered (no-op)
   * @note Does not affect handlers for other event types
   *
   * @see on() to register handlers
   * @see off() to remove specific handler
   */
  template<typename EventType>
  void off_all()
  {
    std::type_index type_id(typeid(EventType));
    _handlers.erase(type_id);
  }

  /**
   * @brief Emit an event from JSON representation.
   *
   * Deserializes an event from JSON using Event::from_json(), then emits it
   * to all registered handlers. This is the primary method for network-received
   * events or configuration-driven events.
   *
   * The event type must provide:
   * @code
   * static Event from_json(const nlohmann::json& j);
   * @endcode
   *
   * @tparam Event Event type (must satisfy event_type concept)
   * @param j JSON object containing event data
   *
   * @throws std::exception If from_json() fails or JSON is malformed
   *
   * @note Internally calls emit(event) after deserialization
   * @note JSON structure must match Event::from_json() expectations
   *
   * @code
    // TODO: real example
   * @endcode
   *
   * @see emit(const Event&) for direct type-safe emission
   * @see emit<Event>(const byte*, std::size_t) for binary emission
   */
  void emit(Registry&, std::string const& name, JsonObject const& args);

  /**
   * @brief Emit an event, invoking all registered handlers.
   *
   * Executes every handler registered via on() for this event type, in the
   * order they were registered. Handlers receive the event object and a
   * reference to this Registry.
   *
   * This is the type-safe emission method - the event is constructed directly
   * in code and passed by reference. For JSON or binary emission, see the
   * overloaded variants.
   *
   * @tparam Event Event type (must satisfy event_type concept)
   * @param event Event object to pass to handlers
   *
   * @note Handlers execute immediately in registration order
   * @note If a handler throws, subsequent handlers may not execute
   * @note Handlers can emit new events (recursive emission supported)
   *
   * @code
   * // Notify all handlers that player took damage
   * registry.emit(PlayerDamaged{
   *   .player_id = player_entity,
   *   .damage = 25,
   *   .damage_type = DamageType::Fire
   * });
   * @endcode
   *
   * @code
   * registry.on<EnemyDestroyed>([](const EnemyDestroyed& e, Registry& r) {
   *   // Handler can emit new events
   *   r.emit(SpawnPickup{.position = e.position});
   *   r.emit(AddScore{.points = e.points});
   * });
   *
   * // This triggers a cascade
   * registry.emit(EnemyDestroyed{...});
   * @endcode
   *
   * @code
   * void apply_damage(Registry& r, Entity target, int damage) {
   *   auto& healths = r.get_components<Health>();
   *   if (healths[target]) {
   *     healths[target]->current -= damage;
   *
   *     if (healths[target]->current <= 0) {
   *       r.emit(EntityDied{.entity = target});
   *     }
   *   }
   * }
   * @endcode
   *
   * @see on() to register handlers
   * @see emit<Event>(const nlohmann::json&) for JSON emission
   * @see emit<Event>(const byte*, std::size_t) for binary emission
   */
  template<typename EventType, typename... Args>
  void emit(Args&&... args)
  {
    std::type_index type_id(typeid(EventType));
    if (!_handlers.contains(type_id)) {
      return;
    }

    EventType event(std::forward<Args>(args)...);

    auto handlers_copy =
        std::any_cast<std::vector<Event<EventType>>>(_handlers.at(type_id));

    for (auto const& handler : handlers_copy) {
      handler(event);
    }
  }

  /**
   * @brief Emit an event from binary representation.
   *
   * Deserializes an event from binary data using Event::from_bytes(), then
   * emits it to all registered handlers. This is the most efficient method
   * for network transmission.
   *
   * The event type must provide:
   * @code
   * static Event from_bytes(const byte* data, std::size_t length);
   * @endcode
   *
   * @tparam Event Event type (must satisfy event_type concept)
   * @param data Pointer to binary event data
   * @param length Number of bytes in data
   *
   * @throws std::exception If from_bytes() fails or data is malformed
   *
   * @note Internally calls emit(event) after deserialization
   * @note More efficient than JSON for network transmission
   * @note Binary format must match Event::from_bytes() expectations
   *
   * @code
    // TODO: real example
   * @endcode
   *
   * @see emit(const Event&) for direct type-safe emission
   * @see emit<Event>(const nlohmann::json&) for JSON emission
   */
  void emit(std::string const& name, ByteArray const& data);

  template<event Event>
  std::string get_event_key()
  {
    return this->_index_getter.at_first(typeid(Event));
  }

  ByteArray convert_event_entity(
      std::string const& id,
      ByteArray const& event,
      std::unordered_map<Registry::Entity, Registry::Entity> const& map);

  void delete_all();

private:
  // ============================================================================
  // EVENT BUILDERS & TEMPLATES
  // ============================================================================

  // Infrastructure for JSON-based event construction and entity templating.
  //
  // Event Builders:
  // Event builders enable creating event objects from JSON. Each event type
  // can register a builder function that constructs the event from JSON params.
  // This is used for network events and configuration-driven events.
  //
  // Templates:
  // Templates are JSON configurations for entity prefabs. Store reusable entity
  // definitions and instantiate them multiple times. Common for enemy types,
  // powerups, UI elements, etc.
  //
  // Event Builder Flow:
  // 1. add_event_builder<Event>() - Register JSON->Event constructor
  // 2. get_event_with_id("event_name", json) - Build event from JSON
  // 3. Event is serialized to binary for network transmission
  //
  // Template Flow:
  // 1. add_template("enemy_basic", json_config) - Register prefab
  // 2. get_template("enemy_basic") - Retrieve config
  // 3. EntityLoader::load_entity(config) - Instantiate entity
  //
  // Network Integration:
  // Event builders are automatically registered by on() to enable JSON
  // emission. The builder constructs events from network-received JSON
  // payloads.
  /**
   * @brief Register a JSON builder for an event type.
   *
   * Creates infrastructure for constructing events from JSON. This enables:
   * - JSON-based event emission via emit<Event>(json)
   * - Network event reception and deserialization
   * - Configuration-driven event triggering
   *
   * The event type T must have a constructor accepting (Registry&, JsonObject).
   * This constructor should parse JSON and initialize event fields.
   *
   * This method is typically called automatically by on() when registering
   * event handlers. Manual calls are rarely needed.
   *
   * @tparam T Event type (must satisfy event concept)
   *
   * @note Event must have constructor: T(Registry&, JsonObject const&)
   * @note Event must have to_bytes() method for serialization
   * @note Usually called automatically by on() - manual use is advanced
   *
   * @see on() which calls this automatically
   * @see get_event_with_id() to construct events from JSON
   * @see emit<Event>(const nlohmann::json&) for JSON emission
   */
  template<event T>
  void add_event_builder()
  {
    std::type_index type_id(typeid(T));

    _builders.insert_or_assign(
        type_id,
        std::function<std::any(Registry&, JsonObject const&)>(
            [](Registry& r, JsonObject const& e) -> std::any
            { return T(r, e); }));

    _invokers.insert_or_assign(
        type_id,
        [](const std::any& handlers_any, const std::any& event_any)
        {
          auto& handlers =
              std::any_cast<const std::vector<Event<T>>&>(handlers_any);
          auto& event = std::any_cast<const T&>(event_any);
          for (auto const& handler : handlers) {
            handler(event);
          }
        });

    _json_builder.insert_or_assign(type_id,
                                   [](Registry& r, JsonObject const& params)
                                   { return T(r, params).to_bytes(); });
  }

  template<event EventType>
  void on(std::function<void(const EventType&)> handler, size_t precision = 1)
  {
    std::type_index type_id(typeid(EventType));

    // Ensure the vector exists BEFORE trying to get a reference
    if (!_handlers.contains(type_id)) {
      _handlers.emplace(type_id, std::vector<Event<EventType>>());
    }

    Event<EventType> tmp(handler, precision);

    // Now safe to get reference using .at() which throws on missing key
    auto& handlers =
        std::any_cast<std::vector<Event<EventType>>&>(_handlers.at(type_id));
    auto insert_pos = std::upper_bound(handlers.begin(), handlers.end(), tmp);
    handlers.insert(insert_pos, std::move(tmp));
  }

  std::unordered_map<
      std::string,
      std::function<ByteArray(
          ByteArray const&,
          std::unordered_map<Registry::Entity, Registry::Entity> const&)>>
      _entity_converter;

  std::unordered_map<std::string, std::function<void(ByteArray const&)>>
      _byte_emitter;

  TwoWayMap<std::type_index, std::string> _index_getter;
  std::unordered_map<std::type_index, std::any> _builders;

  std::unordered_map<std::type_index, std::any> _handlers;
  std::unordered_map<std::type_index,
                     std::function<ByteArray(Registry&, JsonObject const&)>>
      _json_builder;

  std::unordered_map<std::type_index,
                     std::function<void(const std::any&, const std::any&)>>
      _invokers;
};
