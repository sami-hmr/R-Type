/**
 * @file Registry.hpp
 * @brief Central hub of the ECS architecture managing entities, components,
 * systems, and events
 *
 * The Registry is the core orchestrator of the Entity-Component-System pattern.
 * It:
 * - Allocates and tracks entity IDs with efficient recycling
 * - Stores components in type-erased sparse arrays
 * - Executes systems in priority order
 * - Routes events through type-safe handlers
 * - Manages dynamic data bindings between components
 * - Coordinates scene management and transitions
 * - Provides hook-based runtime introspection
 * - Handles network entity ID remapping
 *
 * @section reg_architecture Architecture
 *
 * @subsection reg_entities Entity Management
 *
 * Entities are lightweight IDs (size_t) that serve as indices into component
 * arrays. The Registry maintains:
 * - Active entity counter (_max)
 * - Dead entity queue for ID recycling (_dead_entities)
 * - Entities pending deletion (_entities_to_kill)
 *
 * Lifecycle:
 * 1. spawn_entity() - Allocate new or recycled ID
 * 2. add_component() - Attach data to entity
 * 3. Systems process entities each frame
 * 4. kill_entity() - Mark for deletion
 * 5. process_entity_deletions() - Clean up at frame end
 *
 * @subsection reg_components Component Storage
 *
 * Components are stored in SparseArray<T> containers, one per component type.
 * Registration creates type-erased storage with helper functions:
 *
 * @code
 * // Registration
 * registry.register_component<Position>("Position");
 *
 * // Storage (_components):
 * {
 *   typeid(Position) -> SparseArray<Position> (as std::any)
 * }
 *
 * // Helper maps:
 * _delete_functions:  typeid -> lambda to erase component
 * _emplace_functions: typeid -> lambda to insert from bytes
 * _state_getters:     typeid -> lambda to serialize state
 * _index_getter:      typeid <-> "Position" string lookup
 * @endcode
 *
 * @subsection reg_systems System Execution
 *
 * Systems are priority-ordered functions that process components:
 *
 * @code
 * // High priority runs first
 * registry.add_system<Position, Velocity>(
 *   [](Registry& r, auto& pos, auto& vel) {
 *     // Move entities
 *   },
 *   priority: 10
 * );
 *
 * // Lower priority runs later
 * registry.add_system<Position, Sprite>(
 *   [](Registry& r, auto& pos, auto& spr) {
 *     // Render entities
 *   },
 *   priority: 100
 * );
 *
 * // Execution order maintained in _frequent_systems vector
 * registry.run_systems(); // Calls in priority order
 * @endcode
 *
 * @subsection reg_events Event System
 *
 * Type-safe event emission and handling with network support:
 *
 * @code
 * // Register handler
 * HandlerId id = registry.on<CollisionEvent>(
 *   "collision",
 *   [](const CollisionEvent& e) {
 *     // Handle collision
 *   }
 * );
 *
 * // Emit event
 * registry.emit<CollisionEvent>(entity_a, entity_b);
 *
 * // Emit from JSON (network)
 * registry.emit("collision", json_params);
 *
 * // Emit from bytes (network)
 * registry.emit("collision", byte_array);
 *
 * // Unregister
 * registry.off<CollisionEvent>(id);
 * @endcode
 *
 * @subsection reg_bindings Dynamic Bindings
 *
 * Automatic field synchronization between components:
 *
 * @code
 * // In JSON config:
 * {
 *   "follower": {
 *     "target_pos": "#Leader:pos"  // Bind to Leader's position
 *   }
 * }
 *
 * // Binding registered during component construction:
 * registry.register_binding<Follower, Vector2D>(
 *   follower_entity,
 *   "target_pos",
 *   "Leader:pos"
 * );
 *
 * // Every frame:
 * registry.update_bindings(); // Follower.target_pos = Leader.pos
 * @endcode
 *
 * @subsection reg_scenes Scene Management
 *
 * Multi-scene support for menus, gameplay, overlays:
 *
 * @code
 * // Define scenes
 * registry.add_scene("Menu", SceneState::MAIN);
 * registry.add_scene("Game", SceneState::DISABLED);
 * registry.add_scene("HUD", SceneState::ACTIVE);
 *
 * // Initialize
 * registry.init_scene_management();
 * registry.setup_scene_systems();
 *
 * // Transition
 * registry.set_current_scene("Game");
 * registry.remove_current_scene("Menu");
 *
 * // Entities belong to scenes via Scene component
 * // Zipper filters systems by active scenes
 * @endcode
 *
 * @subsection reg_hooks Hook System
 *
 * Runtime component field access by string name:
 *
 * @code
 * // Register hookable component
 * registry.register_hook<Player>("Player", player_entity);
 *
 * // Access field
 * auto health_ref = registry.get_hooked_value<int>("Player", "health");
 * if (health_ref) {
 *   int current_health = health_ref->get();
 * }
 *
 * // Used by JSON loading and dynamic bindings
 * @endcode
 *
 * @subsection reg_network Network Support
 *
 * Entity ID remapping for client-server synchronization:
 *
 * @code
 * // Server sends event
 * CollisionEvent server_event{entity_a: 5, entity_b: 7};
 * ByteArray bytes = server_event.to_bytes();
 *
 * // Client receives and remaps
 * std::unordered_map<Entity, Entity> map{{5, 12}, {7, 15}};
 * ByteArray remapped = registry.convert_event_entity("collision", bytes, map);
 *
 * // Emit with local entity IDs
 * registry.emit("collision", remapped);
 * @endcode
 *
 * @section reg_concepts Type Constraints
 *
 * Registry uses C++20 concepts to enforce type requirements at compile-time.
 *
 * @see bytable concept in Byte.hpp
 * @see entity_convertible in EventMacros.hpp
 * @see json_buildable in EventConcept.hpp
 * @see hookable in HookConcept.hpp
 */

#pragma once

#include <algorithm>
#include <any>
#include <cstddef>
#include <functional>
#include <iostream>
#include <optional>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Clock.hpp"
#include "Json/JsonParser.hpp"
#include "SparseArray.hpp"
#include "TwoWayMap.hpp"
#include "ecs/ComponentState.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/Systems.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookConcept.hpp"
#include "plugin/events/EventConcept.hpp"

/**
 * @concept component
 * @brief Requires a type to be serializable and entity-convertible for network
 * sync
 *
 * A valid component type must:
 * - Satisfy bytable (provide to_bytes() and from-bytes constructor)
 * - Satisfy entity_convertible (provide change_entity() method)
 *
 * @tparam T The type to check
 *
 * @details
 * Components must be network-transmittable, requiring:
 * - Binary serialization for efficient transmission
 * - Entity ID remapping for client-server synchronization
 *
 * @example
 * @code
 * struct Position {
 *   double x, y;
 *
 *   // Bytable requirement
 *   DEFAULT_BYTE_CONSTRUCTOR(Position, ...)
 *   DEFAULT_SERIALIZE(...)
 *
 *   // Entity-convertible requirement
 *   CHANGE_ENTITY_DEFAULT  // No entities to remap
 * };
 *
 * static_assert(component<Position>);
 * @endcode
 *
 * @see bytable in Byte.hpp
 * @see entity_convertible in EventMacros.hpp
 */

template<typename T>
concept component = bytable<T> && entity_convertible<T>;

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
 * @example
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

template<typename T>
concept event = bytable<T> && entity_convertible<T> && json_buildable<T>;

/**
 * @brief The Registry class is the core of the ECS (Entity-Component-System)
 * architecture.
 *
 * It manages entities, their associated components, and the systems that
 * operate on them.
 *
 * @example Basic Usage
 * @code
 * Registry registry;
 *
 * // Register components
 *
 * REGISTER_COMPONENT(Position)
 * REGISTER_COMPONENT(Velocity)
 *
 * // Spawn entity
 * auto player = registry.spawn_entity();
 * registry.add_component(player, Position{100, 200});
 * registry.add_component(player, Velocity{5, 0});
 *
 * // Add system
 * registry.add_system<Position, Velocity>(
 *   [](Registry& r, auto& pos, auto& vel) {
 *     for (auto&& [p, v] : Zipper(pos, vel)) {
 *       p.x += v.speed.x;
 *       p.y += v.speed.y;
 *     }
 *   }
 * );
 *
 * // Game loop
 * while (running) {
 *   registry.run_systems();  // Update, process deletions, tick clock
 * }
 * @endcode
 * @note Registry is non-copyable due to internal state complexity
 * @note Thread-safety is not guaranteed - single-threaded use recommended
 */
class Registry
{
public:
  /**
   * @brief Type alias for an entity identifier.
   *
   * Entities are lightweight indices into component arrays. Valid entity IDs
   * start at 0 and increment. Deleted entity IDs are recycled.
   */
  using Entity = std::size_t;

  /**
   * @brief Type alias for event handler identifiers
   *
   * Handler IDs are unique UUIDs generated when registering event handlers.
   * Used to unregister specific handlers via off().
   */
  using HandlerId = std::size_t;

  /**
   * @brief Registers a bytable component type with a string identifier.
   *
   * @tparam Component The type of the component to register (must satisfy
   * bytable concept).
   * @param string_id The string identifier for this component type.
   * @return SparseArray<Component>& A reference to the sparse array of the
   * registered component type.
   *
   * @note Must be called before using the component type
   * @note String ID must be unique across all component types
   * @note Registration is idempotent - re-registering replaces existing
   * @see // TODO: Mention Macros
   */

  // ========================================================================
  // COMPONENT REGISTRATION AND ACCESS
  // ========================================================================
  template<component Component>
  SparseArray<Component>& register_component(std::string const& string_id)
  {
    std::type_index ti(typeid(Component));

    this->_components.insert_or_assign(ti, SparseArray<Component>());
    SparseArray<Component>& comp = this->get_components<Component>();

    this->_delete_functions.insert_or_assign(
        ti, [&comp](Entity const& e) { comp.erase(e); });
    this->_emplace_functions.insert_or_assign(
        ti,
        [&comp](Entity const& e, ByteArray const& bytes)
        { comp.insert_at(e, bytes); });
    this->_comp_entity_converters.insert_or_assign(
        string_id,
        [](ByteArray const& b, std::unordered_map<Entity, Entity> const& map)
        { return Component(b).change_entity(map).to_bytes(); });
    this->_state_getters.insert_or_assign(
        ti,
        [&comp, string_id]()
        {
          ComponentState r(string_id);
          for (std::size_t i = 0; i < comp.size(); i++) {
            if (comp[i]) {
              r.comps.emplace_back(i, comp[i]->to_bytes());
            }
          }
          return r;
        });
    this->_index_getter.insert(ti, string_id);
    return comp;
  }

  /**
   * @brief Get the components object
   *
   * @tparam Component The type of the component to retrieve.
   * @return SparseArray<Component>& A reference to the sparse array of the
   * specified component type.
   *
   * @throws std::bad_any_cast if component type not registered
   * @throws std::out_of_range if component type not found
   *
   * @note Component must be registered via register_component() first
   * @note Make sure to use Zipper or ZipperIndex
   */
  template<class Component>
  SparseArray<Component>& get_components()
  {
    return std::any_cast<SparseArray<Component>&>(
        this->_components.at(std::type_index(typeid(Component))));
  }

  /**
   * Same as SparseArray<Component>& get_components() but const
   * @copydoc SparseArray<Component>& get_components()
   */
  template<class Component>
  SparseArray<Component> const& get_components() const
  {
    return std::any_cast<const SparseArray<Component>&>(
        this->_components.at(std::type_index(typeid(Component))));
  }

  /**
   * @brief Checks if an entity has all specified components
   *
   * @tparam Component... One or more component types to check
   * @param e The entity to check
   * @return true if entity has all specified components, false otherwise
   *
   * @details
   * Uses fold expression to check all component types in a single call.
   * Returns false if:
   * - Entity ID is out of bounds for any component array
   * - Any component is not present (nullopt)
   *
   * @example Single Component
   * @code
   * if (registry.has_component<Position>(entity)) {
   *   auto& pos = registry.get_components<Position>()[entity];
   *   // Use pos
   * }
   * @endcode
   *
   * @example Multiple Components
   * @code
   * // Check if entity is a moving sprite
   * if (registry.has_component<Position, Velocity, Sprite>(entity)) {
   *   // Entity has all three components
   *   process_moving_sprite(entity);
   * }
   * @endcode
   *
   **/
  template<class... Component>
  bool has_component(const Entity& e) const
  {
    return ((
                [this, e]()
                {
                  SparseArray<Component> const& comp =
                      this->get_components<Component>();
                  return (e < comp.size() && comp[e].has_value());
                })()
            && ...);
  }

  // ========================================================================
  // ENTITY MANAGEMENT
  // ========================================================================

  /**
   * @brief Spawns a new entity
   *
   * Allocates a unique entity ID by either:
   * - Recycling a previously deleted entity ID from _dead_entities queue
   * - Incrementing _max counter to create a new ID
   *
   * @return Unique entity identifier
   *
   * @details
   * Entities are lightweight indices. This function only allocates the ID;
   * components must be added separately via add_component() or
   * emplace_component().
   *
   * The recycling mechanism ensures ID space stays compact:
   * - When entities are killed, their IDs go into _dead_entities queue
   * - spawn_entity() prefers recycled IDs over creating new ones
   * - This prevents unbounded ID growth in long-running games
   *
   * @note Entity 0 is valid - entities start from 0
   * @note No maximum entity limit (bounded by size_t max)
   * @note Thread-unsafe - single-threaded use only
   *
   * @example Basic Spawn
   * @code
   * Registry registry;
   *
   * auto player = registry.spawn_entity();    // ID: 0
   * auto enemy1 = registry.spawn_entity();    // ID: 1
   * auto enemy2 = registry.spawn_entity();    // ID: 2
   * @endcode
   *
   * @example With Components
   * @code
   * auto bullet = registry.spawn_entity();
   * registry.add_component(bullet, Position{100, 50});
   * registry.add_component(bullet, Velocity{10, 0});
   * registry.add_component(bullet, Sprite{"bullet.png"});
   * @endcode
   *
   * @example ID Recycling
   * @code
   * auto e1 = registry.spawn_entity();  // 0
   * auto e2 = registry.spawn_entity();  // 1
   * auto e3 = registry.spawn_entity();  // 2
   *
   * registry.kill_entity(e2);           // Mark 1 for deletion
   * registry.process_entity_deletions(); // Delete 1, add to queue
   *
   * auto e4 = registry.spawn_entity();  // 1 (recycled!)
   * auto e5 = registry.spawn_entity();  // 3 (new)
   * @endcode
   *
   * @see kill_entity() to destroy entities
   * @see process_entity_deletions() to recycle IDs
   */
  Entity spawn_entity();

  /**
   * @brief Marks an entity for deletion
   *
   * Queues an entity to be destroyed at the end of the current frame.
   * Actual deletion occurs during process_entity_deletions(), typically
   * called at the end of run_systems().
   *
   * @param e The entity to kill
   *
   * @details
   * Deferred deletion prevents iterator invalidation and mid-system crashes:
   * - Entity added to _entities_to_kill set
   * - Systems can still access the entity this frame
   * - Components removed at frame end
   * - Entity ID recycled for future use
   *
   * Multiple kills of same entity are safe (set deduplicates).
   *
   * @warning Entity remains accessible until process_entity_deletions()
   * @note Killing already-dead entity is safe (no-op)
   *
   * @example Basic Kill
   * @code
   * registry.kill_entity(bullet);
   * // Bullet still exists here
   *
   * registry.run_systems();  // Calls process_entity_deletions()
   * // Bullet deleted now
   * @endcode
   *
   * @example Safe Mid-System Kill
   * @code
   * registry.add_system<Position, Health>([](Registry& r, auto& pos, auto& hp)
   * { for (auto&& [e, p, h] : Zipper(pos, hp)) { if (h.current <= 0) {
   *       r.kill_entity(e.entity());  // Safe - deferred deletion
   *     }
   *   }
   *   // All entities still valid during iteration
   * });
   * @endcode
   *
   * @example Checking Kill Status
   * @code
   * registry.kill_entity(enemy);
   * if (registry.is_entity_dying(enemy)) {
   *   // Don't interact with dying entity
   * }
   * @endcode
   *
   * @see process_entity_deletions() for actual deletion
   * @see is_entity_dying() to check pending deletion
   */
  void kill_entity(Entity const& e);

  /**
   * @brief Checks if an entity is marked for deletion
   *
   * @param e The entity to check
   * @return true if entity is in _entities_to_kill set, false otherwise
   *
   * @details
   * Useful to avoid processing entities that will be deleted soon.
   * Entity remains accessible but should be treated as "dead".
   *
   * @example
   * @code
   * registry.kill_entity(enemy);
   *
   * if (!registry.is_entity_dying(enemy)) {
   *   // Safe to process
   *   process_enemy(enemy);
   * }
   * @endcode
   */
  bool is_entity_dying(Entity const& e) const;

  /**
   * @brief Deletes all entities marked for killing
   *
   * Processes the _entities_to_kill set, removing all components from
   * those entities and recycling their IDs.
   *
   * @details
   * For each entity in _entities_to_kill:
   * 1. Call delete function for every registered component type
   * 2. Add entity ID to _dead_entities queue for recycling
   * 3. Clear _entities_to_kill set
   *
   * This is automatically called at the end of run_systems().
   *
   * @note Generally called automatically - manual use rarely needed
   * @note Safe to call multiple times (clears empty set)
   *
   * @example Manual Use
   * @code
   * // Batch kill entities
   * for (auto enemy : enemies_to_remove) {
   *   registry.kill_entity(enemy);
   * }
   *
   * // Force immediate deletion (unusual)
   * registry.process_entity_deletions();
   * // Entities deleted now instead of at frame end
   * @endcode
   *
   * @example Automatic Use (Normal)
   * @code
   * while (game_running) {
   *   registry.run_systems();
   *   // process_entity_deletions() called automatically here
   * }
   * @endcode
   *
   * @see kill_entity() to mark entities for deletion
   * @see run_systems() which calls this automatically
   */

  void process_entity_deletions();

  // ========================================================================
  // COMPONENT ADDITION AND REMOVAL
  // ========================================================================

  /**
   * @brief Adds a component to an entity by moving
   *
   * Transfers ownership of a component to an entity's component storage.
   * The component is moved into the SparseArray at the entity's index.
   *
   * @tparam Component The component type to add (must satisfy component
   * concept)
   * @param to The entity to receive the component
   * @param c The component to add (will be moved)
   * @return Reference to the inserted component
   *
   * @details
   * Uses perfect forwarding to efficiently move the component into storage.
   * If entity already has this component type, it is replaced.
   *
   * @note Entity must be valid (spawned and not killed)
   * @note Component type must be registered first
   * @note Prefer emplace_component() for in-place construction
   *
   * @example Move Existing Component
   * @code
   * Position pos{100, 200, 1};
   * auto& inserted = registry.add_component(entity, std::move(pos));
   * // pos is now moved-from
   * // inserted is the component in storage
   * @endcode
   *
   * @example Inline Construction
   * @code
   * registry.add_component(player, Position{0, 0});
   * registry.add_component(player, Velocity{5, 5});
   * registry.add_component(player, Health{100, 100});
   * @endcode
   *
   * @example Replace Existing
   * @code
   * registry.add_component(entity, Position{0, 0});
   * // Later...
   * registry.add_component(entity, Position{100, 50}); // Replaces old position
   * @endcode
   *
   * @see emplace_component() for in-place construction
   * @see remove_component() to remove components
   */
  template<component Component>
  typename SparseArray<Component>::Ref add_component(Entity const& to,
                                                     Component&& c)
  {
    return this->get_components<Component>().insert_at(
        to, std::forward<Component>(c));
  }

  /**
   * @brief Constructs a component in-place on an entity
   *
   * Creates a component directly in storage by forwarding constructor
   * arguments. More efficient than add_component() when constructing from
   * parameters.
   *
   * @tparam Component The component type to construct
   * @tparam Params Constructor parameter types (deduced)
   * @param to The entity to receive the component
   * @param p Constructor arguments forwarded to Component constructor
   * @return Reference to the constructed component
   *
   * @details
   * Constructs the component directly in the SparseArray, avoiding temporary
   * objects and move operations. Equivalent to:
   * @code
   * storage[entity] = Component(args...);  // But without temporary
   * @endcode
   *
   * @note More efficient than add_component() for complex components
   * @note Replaces existing component if present
   *
   * @example Basic Construction
   * @code
   * // Direct construction
   * registry.emplace_component<Position>(entity, 100.0, 200.0, 1);
   *
   * // Equivalent to (but more efficient than):
   * registry.add_component(entity, Position(100.0, 200.0, 1));
   * @endcode
   *
   * @example Complex Types
   * @code
   * // Sprite with multiple parameters
   * registry.emplace_component<Sprite>(
   *   entity,
   *   "player.png",           // texture
   *   Rect{0, 0, 32, 32},    // source rect
   *   Vector2D{16, 16},      // origin
   *   Color::WHITE           // tint
   * );
   * @endcode
   *
   * @see add_component() to move existing components
   */
  template<typename Component, typename... Params>
  typename SparseArray<Component>::Ref emplace_component(Entity const& to,
                                                         Params&&... p)
  {
    return this->get_components<Component>().insert_at(
        to, std::forward<Params>(p)...);
  }

  /**
   * @brief Constructs a component from binary data by string ID
   *
   * Deserializes a component from a byte array and attaches it to an entity.
   * Used for network synchronization and state loading.
   *
   * @param to The entity to receive the component
   * @param string_id The component type identifier (from registration)
   * @param bytes Binary representation of the component
   *
   * @details
   * Lookup process:
   * 1. Find type_index via string_id in _index_getter
   * 2. Find emplace function in _emplace_functions
   * 3. Call function to deserialize and insert component
   *
   * If string_id is unknown, prints error to stderr and does nothing.
   *
   * @note Component type must be registered with this string_id
   * @note Byte array must be valid serialized component data
   * @note Used internally by network systems
   *
   * @example Network Receive
   * @code
   * // Server sends component update
   * ByteArray bytes = position_component.to_bytes();
   * network.send(client, "Position", bytes);
   *
   * // Client receives and applies
   * std::string comp_id;
   * ByteArray data;
   * network.receive(comp_id, data);
   * registry.emplace_component(entity, comp_id, data);
   * @endcode
   *
   * @example State Loading
   * @code
   * ComponentState state = load_from_file("save.dat");
   * for (auto& [entity_id, comp_data] : state.comps) {
   *   registry.emplace_component(entity_id, state.id, comp_data);
   * }
   * @endcode
   *
   * @see register_component() which sets up the emplace function
   * @see ComponentState for serialization format
   */
  void emplace_component(Entity const& to,
                         std::string const& string_id,
                         ByteArray const& bytes);

  /**
   * @brief Removes a component from an entity
   *
   * Erases the component at the entity's index in the component storage array.
   * If the entity doesn't have this component, this is a no-op.
   *
   * @tparam Component The component type to remove
   * @param from The entity to remove the component from
   *
   * @details
   * Sets the SparseArray slot to std::nullopt. The entity remains valid
   * and can still have other component types.
   *
   * @note Safe to call on entities without the component
   * @note Component type must be registered (or bad_any_cast thrown)
   * @note Does not kill the entity - only removes one component
   *
   * @example Single Removal
   * @code
   * // Remove velocity to stop movement
   * registry.remove_component<Velocity>(entity);
   * // Entity still exists with other components
   * @endcode
   *
   * @example Conditional Removal
   * @code
   * registry.add_system<Position, Sprite>([](Registry& r, auto& pos, auto& spr)
   * { for (auto&& [e, p, s] : Zipper(pos, spr)) { if (p.x < 0 || p.x >
   * screen_width) {
   *       // Remove offscreen sprites to save rendering
   *       r.remove_component<Sprite>(e.entity());
   *     }
   *   }
   * });
   * @endcode
   *
   * @see kill_entity() to remove all components and delete entity
   * @see add_component() to add components back
   */
  template<typename Component>
  void remove_component(Entity const& from)
  {
    this->get_components<Component>().erase(from);
  }

  /**
   * @brief Adds a system that operates on specified components.
   *
   * @tparam Components: The types of the components the system will operate on.
   * @tparam Function The type of the function representing the system.
   * @param f The function representing the system.
   */
  template<class... Components, typename Function>
  void add_system(Function&& f, std::size_t priority = 0)
  {
    System<> sys([this, f = std::forward<Function>(f)]()
                 { f(*this, this->get_components<Components>()...); },
                 priority);
    auto it = std::upper_bound(
        this->_frequent_systems.begin(), this->_frequent_systems.end(), sys);
    this->_frequent_systems.insert(it, std::move(sys));
  }

  // ========================================================================
  // SYSTEM MANAGEMENT
  // ========================================================================

  /**
   * @brief Registers a system function to run every frame
   *
   * Adds a system to the execution pipeline. Systems are functions that process
   * entities with specific component combinations. They run in priority order
   * during run_systems().
   *
   * @tparam Components Component types the system operates on
   * @tparam Function System function type (deduced)
   * @param f The system function - signature: void(Registry&,
   * SparseArray<Component>&...)
   * @param priority Execution priority (lower = earlier, default: 0)
   *
   * @details
   * System execution:
   * 1. Systems stored in priority order in _frequent_systems
   * 2. run_systems() calls each system with registry and component arrays
   * 3. System iterates entities (typically using Zipper)
   *
   * Priority ordering:
   * - Lower values run first (e.g., priority 0 before priority 100)
   * - Same priority = insertion order
   * - Use priority to enforce dependencies (physics before rendering)
   *
   * @note Systems run every frame in run_systems()
   * @note Component types must be registered before adding systems
   * @note Systems are copyable - lambda captures must be copy-safe
   *
   * @example Basic System
   * @code
   * registry.add_system<Position, Velocity>(
   *   [](Registry& r, auto& positions, auto& velocities) {
   *     for (auto&& [pos, vel] : Zipper(positions, velocities)) {
   *       pos.x += vel.speed.x * r.clock().delta_time();
   *       pos.y += vel.speed.y * r.clock().delta_time();
   *     }
   *   }
   * );
   * @endcode
   *
   * @example Priority System
   * @code
   * // Physics runs first (priority 10)
   * registry.add_system<Position, Velocity>(
   *   [](Registry& r, auto& pos, auto& vel) {
   *     // Update positions
   *   },
   *   10
   * );
   *
   * // Collision detection after physics (priority 20)
   * registry.add_system<Position, Collidable>(
   *   [](Registry& r, auto& pos, auto& col) {
   *     // Check collisions
   *   },
   *   20
   * );
   *
   * @endcode
   *
   * @example Multiple Component Types
   * @code
   * registry.add_system<Position, Velocity, Sprite, Health>(
   *   [](Registry& r, auto& pos, auto& vel, auto& spr, auto& hp) {
   *     // Zipper automatically filters to entities with ALL components
   *     for (auto&& [p, v, s, h] : Zipper(pos, vel, spr, hp)) {
   *       // Only entities with all 4 components
   *       if (h.current <= h.max * 0.3f) {
   *         s.tint = Color::RED;  // Low health = red tint
   *       }
   *     }
   *   }
   * );
   * @endcode
   *
   * @example Entity Access
   * @code
   * registry.add_system<Position, Health>(
   *   [](Registry& r, auto& positions, auto& healths) {
   *     for (auto&& [entity_wrapper, pos, hp] : IndexedZipper(positions,
   * healths)) { if (hp.current <= 0) { r.kill_entity(entity_wrapper.entity());
   * // Access entity ID
   *       }
   *     }
   *   }
   * );
   * @endcode
   *
   * @example Emitting Events
   * @code
   * registry.add_system<Position, Velocity>(
   *   [](Registry& r, auto& pos, auto& vel) {
   *     for (auto&& [e, p, v] : IndexedZipper(pos, vel)) {
   *       if (p.x < 0 || p.x > screen_width) {
   *         r.emit<EntityOffscreenEvent>(e.entity(), p.x, p.y);
   *       }
   *     }
   *   }
   * );
   * @endcode
   *
   * @see run_systems() to execute all registered systems
   * @see Zipper for entity iteration
   * @see System class for wrapper details
   */
  void run_systems();

  /**
   * @brief Updates all registered dynamic bindings
   *
   * Synchronizes component fields that are bound to other component values.
   * For each binding, copies the source value to the target field.
   *
   * @details
   * Called automatically at the start of run_systems(). Processes _bindings
   * vector, executing each binding's updater function to copy values.
   *
   * Bindings are created via register_binding() during component construction
   * from JSON with # hook syntax.
   *
   * @note Automatically called by run_systems() - manual call rarely needed
   * @note Order of binding updates matches registration order
   * @note Failed bindings are silently ignored (no error thrown)
   *
   * @example Automatic Update
   * @code
   * // Bindings updated automatically
   * while (running) {
   *   // Leader moves
   *   leader_pos.x += 10;
   *
   *   registry.run_systems();
   *   // update_bindings() called here
   *   // Follower.target_pos now equals Leader.pos
   * }
   * @endcode
   *
   * @example Manual Update (Rare)
   * @code
   * // Force binding sync mid-frame (unusual)
   * leader_position.x = 500;
   * registry.update_bindings();  // Sync immediately
   * // Follower now tracks new position
   * @endcode
   *
   * @see register_binding() to create bindings
   * @see run_systems() which calls this automatically
   * @see clear_bindings() to remove all bindings
   */
  void update_bindings();

  /**
   * @brief Registers a dynamic binding between component fields
   *
   * Creates a live connection where a target field automatically updates to
   * match a source field every frame. Used for data-driven behaviors like
   * followers, turrets tracking targets, or UI elements following entities.
   *
   * @tparam ComponentType Type of component containing the target field
   * @tparam T Type of the field being bound
   * @param entity Entity owning the target component
   * @param field_name Name of the target field (must be hookable)
   * @param source_hook Source identifier in format "ComponentName:fieldName"
   *
   * @details
   * Creates a Binding struct containing:
   * - updater: Lambda that copies source value to target field each frame
   * - serializer: Lambda that serializes target component for network sync
   *
   * The binding is executed every frame during update_bindings().
   *
   * @note Target component type must be hookable (use HOOKABLE macro)
   * @note Source component must exist and be hookable
   * @note Binding survives until clear_bindings() or entity deletion
   * @note Typically called automatically during component JSON construction
   *
   * @example Follower AI
   * @code
   * struct Follower {
   *   Vector2D target_pos;
   *
   *   Follower(Registry& r, JsonObject const& obj, Entity self) {
   *     // JSON: {"target_pos": "#Leader:pos"}
   *     target_pos = get_value<Follower, Vector2D>(
   *       r, obj, self, "target_pos"
   *     ).value_or(Vector2D{0, 0});
   *
   *     // register_binding called internally by get_value
   *     // Creates: Follower.target_pos <- Leader.pos binding
   *   }
   *
   *   HOOKABLE(Follower, HOOK(target_pos))
   * };
   *
   * // Every frame:
   * // 1. Leader moves to new position
   * // 2. update_bindings() executes
   * // 3. Follower.target_pos = Leader.pos (automatic!)
   * @endcode
   *
   * @example Turret Tracking
   * @code
   * struct Turret {
   *   Vector2D aim_target;
   *   float rotation;
   *
   *   HOOKABLE(Turret, HOOK(aim_target), HOOK(rotation))
   * };
   *
   * // Bind turret aim to player position
   * registry.register_binding<Turret, Vector2D>(
   *   turret_entity,
   *   "aim_target",
   *   "Player:pos"
   * );
   *
   * // Turret automatically aims at player position every frame
   * @endcode
   *
   * @example Health Bar UI
   * @code
   * struct HealthBar {
   *   int current_value;
   *   HOOKABLE(HealthBar, HOOK(current_value))
   * };
   *
   * // Bind UI to player health
   * registry.register_binding<HealthBar, int>(
   *   health_bar_entity,
   *   "current_value",
   *   "Player:health.current"
   * );
   *
   * // Health bar automatically reflects player damage
   * @endcode
   *
   * @see update_bindings() which executes bindings
   * @see get_value() which calls this for # hooks
   * @see HOOKABLE macro to make components bindable
   * @see Hooks.hpp for hook syntax details
   */
  template<component ComponentType, typename T>
  void register_binding(Entity entity,
                        std::string const& field_name,
                        std::string const& source_hook)
  {
    std::type_index ti(typeid(ComponentType));

    std::function<void()> updater = [this, entity, field_name, source_hook]()
    {
      try {
        std::string comp = source_hook.substr(0, source_hook.find(':'));
        std::string value = source_hook.substr(source_hook.find(':') + 1);
        auto ref = this->get_hooked_value<T>(comp, value);

        if (ref.has_value()) {
          auto& components = this->get_components<ComponentType>();
          if (entity < components.size() && components[entity].has_value()) {
            auto& target_comp = components[entity].value();

            auto& hook_map = ComponentType::hook_map();
            if (hook_map.contains(field_name)) {  // TODO: PROPER ERROR HANDLING
              std::any field_any = hook_map.at(field_name)(target_comp);
              auto field_ref_wrapper =
                  std::any_cast<std::reference_wrapper<T>>(field_any);
              field_ref_wrapper.get() = ref.value().get();
            }
          }
        }
      } catch (...) {  // NOLINT  TODO: catch correctly
      }
    };

    std::function<ByteArray()> serializer = [this, entity]()
    {
      auto& components = this->get_components<ComponentType>();
      if (entity < components.size() && components[entity].has_value()) {
        return components[entity]->to_bytes();
      }
      return ByteArray {};
    };

    _bindings.emplace_back(entity,
                           ti,
                           field_name,
                           source_hook,
                           std::move(updater),
                           std::move(serializer));
  }

  /**
   * @brief Removes all registered bindings
   *
   * Clears the _bindings vector, stopping all automatic field synchronization.
   * Typically used when transitioning between game states or loading new
   * scenes.
   *
   * @details
   * After clearing:
   * - Bound fields stop updating automatically
   * - Field values remain at last synchronized value
   * - New bindings can be registered normally
   *
   * @note Does not affect component data - only removes update connections
   * @note Bindings are not automatically re-created on next frame
   *
   * @example Scene Transition
   * @code
   * void load_new_scene(Registry& r) {
   *   // Clear old scene
   *   r.clear_bindings();  // Remove follower AI, turret tracking, etc.
   *
   *   // Kill old entities
   *   for (auto e : old_scene_entities) {
   *     r.kill_entity(e);
   *   }
   *
   *   // Load new scene
   *   load_entities_from_json(r, "new_scene.json");
   *   // New bindings registered during entity loading
   * }
   * @endcode
   *
   * @example Gameplay State Change
   * @code
   * void enter_pause_menu(Registry& r) {
   *   r.clear_bindings();  // Stop all dynamic behaviors
   *   // Game entities frozen
   * }
   *
   * void resume_game(Registry& r) {
   *   // Bindings must be re-registered if needed
   *   setup_gameplay_bindings(r);
   * }
   * @endcode
   *
   * @see register_binding() to create new bindings
   * @see update_bindings() which processes bindings
   */
  void clear_bindings();

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
   * @example Basic Handler Registration
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
   * @example Multiple Handlers
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
   * @example Handler with State Modification
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
  HandlerId on(std::string const& name,
               std::function<void(const EventType&)> handler)
  {
    std::type_index type_id(typeid(EventType));
    _events_index_getter.insert(type_id, name);
    if (!_event_entity_converters.contains(name)) {
      _event_entity_converters.insert_or_assign(
          name,
          [](ByteArray const& b, std::unordered_map<Entity, Entity> const& map)
          { return EventType(b).change_entity(map).to_bytes(); });
    }

    if (!_byte_event_emitter.contains(name)) {
      _byte_event_emitter.insert_or_assign(
          name,
          [this](ByteArray const& data)
          { return this->emit<EventType>(EventType(data)); });
    }

    this->add_event_builder<EventType>();

    return this->on<EventType>(handler);
  }

  /**
   * @brief Unregister a specific event handler by ID.
   *
   * Removes the handler that was registered with the given ID. The ID is
   * returned by on() when registering the handler.
   *
   * After unregistration, the handler will not execute when the event is
   * emitted. Other handlers for the same event type remain active.
   *
   * @tparam Event Event type (must satisfy event_type concept)
   * @param id Handler ID returned by on()
   *
   * @note Safe to call with invalid/already-removed IDs (no-op)
   * @note Does not affect other handlers for the same event type
   *
   * @example Temporary Handler
   * @code
   * // Register handler for one-time use
   * auto id = registry.on<GameStarted>(
   *   [](const GameStarted& e, Registry& r) {
   *     show_intro_cinematic();
   *   }
   * );
   *
   * // After cinematic plays, remove handler
   * registry.off<GameStarted>(id);
   * @endcode
   *
   * @example Conditional Handler Management
   * @code
   * class DebugSystem {
   *   std::string debug_handler_id;
   *
   * public:
   *   void enable_debug_mode(Registry& r) {
   *     debug_handler_id = r.on<EntitySpawned>(
   *       [](const EntitySpawned& e, Registry& r) {
   *         std::cout << "DEBUG: Entity " << e.id << " spawned\n";
   *       }
   *     );
   *   }
   *
   *   void disable_debug_mode(Registry& r) {
   *     r.off<EntitySpawned>(debug_handler_id);
   *   }
   * };
   * @endcode
   *
   * @see on() to register handler
   * @see off_all() to remove all handlers
   */
  template<typename EventType>
  bool off(HandlerId handler_id)
  {
    std::type_index type_id(typeid(EventType));
    if (!_event_handlers.contains(type_id)) {
      return false;
    }

    auto& handlers = std::any_cast<
        std::unordered_map<HandlerId, std::function<void(const EventType&)>>&>(
        _event_handlers[type_id]);

    if (!handlers.contains(handler_id)) {
      return false;
    }

    handlers.erase(handler_id);
    return true;
  }

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
    _event_handlers.erase(type_id);
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
   * @example Configuration-Driven Events
   * @code
    // TODO: real example
   * @endcode
   *
   * @see emit(const Event&) for direct type-safe emission
   * @see emit<Event>(const byte*, std::size_t) for binary emission
   */
  void emit(std::string const& name, JsonObject const& args);

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
   * @example Direct Event Emission
   * @code
   * // Notify all handlers that player took damage
   * registry.emit(PlayerDamaged{
   *   .player_id = player_entity,
   *   .damage = 25,
   *   .damage_type = DamageType::Fire
   * });
   * @endcode
   *
   * @example Event Chain
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
   * @example Conditional Emission
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
    if (!_event_handlers.contains(type_id)) {
      return;
    }

    EventType event(std::forward<Args>(args)...);

    auto handlers_copy = std::any_cast<
        std::unordered_map<HandlerId, std::function<void(const EventType&)>>>(
        _event_handlers.at(type_id));

    for (auto const& [id, handler] : handlers_copy) {
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
   * @example Network Packet Reception
   * @code
    // TODO: real example
   * @endcode
   *
   * @see emit(const Event&) for direct type-safe emission
   * @see emit<Event>(const nlohmann::json&) for JSON emission
   */
  void emit(std::string const& name, ByteArray const& data);

  // ============================================================================
  // SCENE MANAGEMENT
  // ============================================================================
  //
  // Multi-scene system for organizing entities into logical layers.
  //
  // Scenes allow you to organize entities into separate contexts (menu,
  // gameplay, HUD, pause screen, etc.). Each scene has a state (MAIN, ACTIVE,
  // DISABLED) that controls whether its entities are processed by systems.
  //
  // Scene States:
  // - MAIN: Primary active scene (only one allowed, typically gameplay)
  // - ACTIVE: Active but not primary (overlays, HUD, UI layers)
  // - DISABLED: Scene exists but entities are not processed
  //
  // System Iteration:
  // The Zipper class automatically filters entities - systems only iterate over
  // entities in MAIN and ACTIVE scenes. DISABLED scenes are skipped entirely.
  //
  // Common Patterns:
  // - Gameplay + HUD: Set gameplay as MAIN, HUD as ACTIVE
  // - Menu switching: Disable old MAIN, enable new scene as MAIN
  // - Pause overlay: Set pause menu as ACTIVE over gameplay MAIN
  //
  // Scene Lifecycle:
  // 1. add_scene() - Register scene with initial state
  // 2. set_current_scene() - Activate scene (sets to MAIN or ACTIVE)
  // 3. remove_current_scene() - Deactivate specific scene
  // 4. remove_all_scenes() - Clear all active scenes

  /**
   * @brief Register a scene with the given name and initial state.
   *
   * Creates a scene entry in the Registry's scene management system. The scene
   * can later be activated/deactivated with set_current_scene() and
   * remove_current_scene().
   *
   * This does NOT create entities - it only registers the scene metadata.
   * Entities are associated with scenes via the Scene component, typically
   * added during EntityLoader::load_entity().
   *
   * @param scene_name Unique identifier for the scene
   * @param state Initial activation state (MAIN, ACTIVE, or DISABLED)
   *
   * @note If scene already exists, this updates its state
   * @note Scene names must be unique
   * @note Call init_scene_management() before using scenes
   *
   * @example Initial Scene Setup
   * @code
   * registry.init_scene_management();
   * registry.add_scene("menu", SceneState::MAIN);
   * registry.add_scene("hud", SceneState::ACTIVE);
   * registry.add_scene("gameplay", SceneState::DISABLED);
   * @endcode
   *
   * @example Dynamic Scene Registration
   * @code
   * void load_level(Registry& r, const std::string& level_name) {
   *   // Register level scene as disabled initially
   *   r.add_scene(level_name, SceneState::DISABLED);
   *
   *   // Load level entities (they get Scene component automatically)
   *   EntityLoader loader(r);
   *   loader.load_entity("level_config.json");
   *
   *   // Activate when ready
   *   r.set_current_scene(level_name);
   * }
   * @endcode
   *
   * @see init_scene_management() must be called first
   * @see set_current_scene() to activate the scene
   * @see SceneState enum for state meanings
   */
  void add_scene(std::string const& scene_name, SceneState state);

  /**
   * @brief Initialize the scene management system.
   *
   * Must be called before using any scene-related methods. Sets up internal
   * data structures for scene tracking and filtering.
   *
   * @note Call this once during Registry initialization
   * @note Safe to call multiple times (idempotent)
   *
   * @example Registry Setup
   * @code
   * Registry registry;
   * registry.init_scene_management();  // Enable scenes
   * registry.add_scene("menu", SceneState::MAIN);
   * @endcode
   *
   * @see add_scene() to register scenes
   * @see setup_scene_systems() for automatic scene management
   */
  void init_scene_management();

  /**
   * @brief Setup automatic scene management systems.
   *
   * Registers systems that automatically handle scene transitions, entity
   * filtering, and scene state updates. This is optional but recommended
   * for automatic scene behavior.
   *
   * @note Call after init_scene_management()
   * @note Adds systems to the Registry's system pipeline
   *
   * @example Complete Scene Setup
   * @code
   * registry.init_scene_management();
   * registry.setup_scene_systems();  // Enable automatic scene handling
   * registry.add_scene("gameplay", SceneState::MAIN);
   * @endcode
   *
   * @see init_scene_management() must be called first
   */
  void setup_scene_systems();

  /**
   * @brief Activate a scene, making its entities visible to systems.
   *
   * Sets the scene to MAIN or ACTIVE state (implementation-dependent).
   * Entities with this scene's Scene component will be processed by systems.
   *
   * If another scene is currently MAIN and this scene is set to MAIN, the
   * previous MAIN scene typically becomes ACTIVE or DISABLED.
   *
   * @param scene_name Name of the scene to activate
   *
   * @note Scene must be registered with add_scene() first
   * @note Entities in the scene become visible to system iteration
   * @note Only one scene should be MAIN at a time
   *
   * @throws std::exception If scene_name was not registered
   *
   * @example Scene Transition
   * @code
   * // Switch from menu to gameplay
   * registry.remove_current_scene("menu");
   * registry.set_current_scene("gameplay");
   * @endcode
   *
   * @example Overlay Scene
   * @code
   * // Show pause menu over gameplay
   * // (both scenes active - systems process both)
   * registry.set_current_scene("gameplay");  // MAIN
   * registry.set_current_scene("pause_menu");  // ACTIVE overlay
   * @endcode
   *
   * @example Level Loading
   * @code
   * void start_level(Registry& r, const std::string& level) {
   *   // Hide menu
   *   r.remove_current_scene("menu");
   *
   *   // Show gameplay and HUD
   *   r.set_current_scene(level);  // Gameplay scene
   *   r.set_current_scene("hud");  // HUD overlay
   * }
   * @endcode
   *
   * @see add_scene() to register scenes
   * @see remove_current_scene() to deactivate
   * @see get_current_scene() to query active scenes
   */
  void set_current_scene(std::string const& scene_name);

  /**
   * @brief Deactivate a specific scene.
   *
   * Sets the scene to DISABLED state. Entities in this scene will no longer
   * be processed by systems until the scene is reactivated with
   * set_current_scene().
   *
   * @param scene_name Name of the scene to deactivate
   *
   * @note Safe to call with non-existent scene names (no-op)
   * @note Does not delete the scene or its entities
   * @note Scene can be reactivated later with set_current_scene()
   *
   * @example Menu to Gameplay Transition
   * @code
   * // User clicks "Start Game"
   * registry.remove_current_scene("main_menu");
   * registry.set_current_scene("gameplay");
   * @endcode
   *
   * @example Pause Menu
   * @code
   * void toggle_pause(Registry& r, bool& paused) {
   *   if (paused) {
   *     r.remove_current_scene("pause_menu");
   *     paused = false;
   *   } else {
   *     r.set_current_scene("pause_menu");
   *     paused = true;
   *   }
   * }
   * @endcode
   *
   * @see set_current_scene() to reactivate
   * @see remove_all_scenes() to deactivate all scenes
   */
  void remove_current_scene(std::string const& scene_name);

  /**
   * @brief Deactivate all scenes.
   *
   * Sets all scenes to DISABLED state. No entities will be processed by
   * systems until scenes are reactivated with set_current_scene().
   *
   * @note Does not delete scenes - they can be reactivated
   * @note Does not delete entities
   * @note Useful for complete state resets
   *
   * @example Game Restart
   * @code
   * void restart_game(Registry& r) {
   *   r.remove_all_scenes();  // Disable everything
   *   r.set_current_scene("main_menu");
   * @endcode
   **/
  void remove_all_scenes();

  /**
   * @brief Get list of currently active scene names.
   *
   * Returns the names of all scenes in MAIN or ACTIVE state. DISABLED scenes
   * are not included. The order may be significant (MAIN typically first).
   *
   * @return Vector of active scene names (may be empty)
   *
   * @note Returned vector is const - use set/remove to modify scenes
   * @note Empty vector means no scenes are active
   *
   * @example Debug Display
   * @code
   * void show_debug_info(Registry& r) {
   *   auto scenes = r.get_current_scene();
   *   std::cout << "Active scenes: ";
   *   for (const auto& scene : scenes) {
   *     std::cout << scene << " ";
   *   }
   *   std::cout << "\n";
   * }
   * @endcode
   *
   * @example Conditional System
   * @code
   * void update_system(Registry& r) {
   *   auto scenes = r.get_current_scene();
   *
   *   // Check if gameplay is active
   *   bool in_game = std::find(scenes.begin(), scenes.end(), "gameplay")
   *                  != scenes.end();
   *
   *   if (in_game) {
   *     // Run gameplay logic
   *   }
   * }
   * @endcode
   *
   * @see set_current_scene() to modify active scenes
   * @see is_in_current_scene() to check specific entity
   */
  std::vector<std::string> const& get_current_scene() const;

  Clock& clock();

  const Clock& clock() const;

  template<hookable T>
  void register_hook(std::string name, Entity const& e)
  {
    this->_hooked_components.insert_or_assign(
        name,
        [this, e](std::string const& key) -> std::optional<std::any>
        {
          auto& array = this->get_components<T>();
          auto& comp = array[e];
          if (!comp.has_value()) {
            return std::nullopt;
          }
          return T::hook_map().at(key)(comp.value());
        });
  }

  template<typename T>
  std::optional<std::reference_wrapper<T>> get_hooked_value(
      std::string const& comp, std::string const& value)
  {
    auto const& tmp = std::any_cast<std::optional<std::any>>(
        this->_hooked_components.at(comp)(value));
    if (!tmp.has_value()) {
      return std::nullopt;
    }
    return std::any_cast<std::reference_wrapper<T>>(tmp.value());
  }

  template<event T>
  void add_event_builder()
  {
    std::type_index type_id(typeid(T));

    _event_builders.insert_or_assign(
        type_id,
        std::function<std::any(Registry&, JsonObject const&)>(
            [](Registry& r, JsonObject const& e) -> std::any
            { return T(r, e); }));

    _event_invokers.insert_or_assign(
        type_id,
        [](const std::any& handlers_any, const std::any& event_any)
        {
          auto& handlers = std::any_cast<
              const std::unordered_map<HandlerId,
                                       std::function<void(const T&)>>&>(
              handlers_any);
          auto& event = std::any_cast<const T&>(event_any);
          for (auto const& [id, handler] : handlers) {
            handler(event);
          }
        });

    _event_json_builder.insert_or_assign(
        type_id,
        [this](JsonObject const& params)
        { return T(*this, params).to_bytes(); });
  }

  ByteArray get_event_with_id(std::string const&, JsonObject const&);

  void add_template(std::string const& name, JsonObject const& config);

  JsonObject get_template(std::string const& name);

  bool is_in_current_cene(Entity e);

  ByteArray convert_event_entity(std::string const& id,
                                 ByteArray const& event,
                                 std::unordered_map<Entity, Entity> const& map);

  ByteArray convert_comp_entity(std::string const& id,
                                ByteArray const& comp,
                                std::unordered_map<Entity, Entity> const& map);

  template<event Event>
  std::string get_event_key()
  {
    return this->_events_index_getter.at_first(typeid(Event));
  }

  template<component Component>
  std::string get_component_key()
  {
    return this->_index_getter.at_first(typeid(Component));
  }

  std::vector<ComponentState> get_state();

private:
  template<typename EventType>
  HandlerId on(std::function<void(const EventType&)> handler)
  {
    std::type_index type_id(typeid(EventType));

    HandlerId handler_id = generate_uuid();

    if (!_event_handlers.contains(type_id)) {
      _event_handlers.insert_or_assign(
          type_id,
          std::unordered_map<HandlerId,
                             std::function<void(const EventType&)>>());
    }

    auto& handlers = std::any_cast<
        std::unordered_map<HandlerId, std::function<void(const EventType&)>>&>(
        _event_handlers[type_id]);

    handlers[handler_id] = std::move(handler);

    return handler_id;
  }

  struct Binding
  {
    Entity target_entity;
    std::type_index target_component;
    std::string target_field;
    std::string source_hook;
    std::function<void()> updater;
    std::function<ByteArray()> serializer;

    Binding(Entity e,
            std::type_index ti,
            std::string tf,
            std::string sh,
            std::function<void()> u,
            std::function<ByteArray()> s)
        : target_entity(e)
        , target_component(ti)
        , target_field(std::move(tf))
        , source_hook(std::move(sh))
        , updater(std::move(u))
        , serializer(std::move(s))
    {
    }
  };

  static HandlerId generate_uuid()
  {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<HandlerId> dis;
    return dis(gen);
  }

  std::unordered_map<std::type_index, std::any> _components;
  std::unordered_map<std::type_index, std::function<void(Entity const&)>>
      _delete_functions;
  std::unordered_map<std::type_index,
                     std::function<void(Entity const&, ByteArray const&)>>
      _emplace_functions;
  std::unordered_map<std::type_index, std::function<ComponentState()>>
      _state_getters;
  TwoWayMap<std::type_index, std::string> _index_getter;

  std::unordered_map<
      std::string,
      std::function<ByteArray(ByteArray const&,
                              std::unordered_map<Entity, Entity> const&)>>
      _event_entity_converters;

  std::unordered_map<std::string, std::function<void(ByteArray const&)>>
      _byte_event_emitter;

  std::unordered_map<
      std::string,
      std::function<ByteArray(ByteArray const&,
                              std::unordered_map<Entity, Entity> const&)>>
      _comp_entity_converters;

  std::unordered_map<std::type_index, std::any> _event_handlers;
  TwoWayMap<std::type_index, std::string> _events_index_getter;
  std::unordered_map<std::type_index, std::any> _event_builders;
  std::unordered_map<std::type_index,
                     std::function<ByteArray(JsonObject const&)>>
      _event_json_builder;
  std::unordered_map<std::type_index,
                     std::function<void(const std::any&, const std::any&)>>
      _event_invokers;

  std::vector<System<>> _frequent_systems;
  std::queue<Entity> _dead_entities;
  std::unordered_set<Entity> _entities_to_kill;
  Clock _clock;
  std::size_t _max = 0;

  std::unordered_map<std::string, SceneState> _scenes;
  std::vector<std::string> _current_scene;

  std::unordered_map<std::string,
                     std::function<std::optional<std::any>(std::string const&)>>
      _hooked_components;
  std::vector<Binding> _bindings;

  std::unordered_map<std::string, JsonObject> _entities_templates;
};
