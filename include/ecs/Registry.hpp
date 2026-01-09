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

class EventManager;

/**
 * @brief The Registry class is the core of the ECS (Entity-Component-System)
 * architecture.
 *
 * It manages entities, their associated components, and the systems that
 * operate on them.
 *
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
   * @code
   * if (registry.has_component<Position>(entity)) {
   *   auto& pos = registry.get_components<Position>()[entity];
   *   // Use pos
   * }
   * @endcode
   *
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
   * @code
   * Registry registry;
   *
   * auto player = registry.spawn_entity();    // ID: 0
   * auto enemy1 = registry.spawn_entity();    // ID: 1
   * auto enemy2 = registry.spawn_entity();    // ID: 2
   * @endcode
   *
   * @code
   * auto bullet = registry.spawn_entity();
   * registry.add_component(bullet, Position{100, 50});
   * registry.add_component(bullet, Velocity{10, 0});
   * registry.add_component(bullet, Sprite{"bullet.png"});
   * @endcode
   *
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
   * @code
   * registry.kill_entity(bullet);
   * // Bullet still exists here
   *
   * registry.run_systems();  // Calls process_entity_deletions()
   * // Bullet deleted now
   * @endcode
   *
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
   * @code
   * Position pos{100, 200, 1};
   * auto& inserted = registry.add_component(entity, std::move(pos));
   * // pos is now moved-from
   * // inserted is the component in storage
   * @endcode
   *
   * @code
   * registry.add_component(player, Position{0, 0});
   * registry.add_component(player, Velocity{5, 5});
   * registry.add_component(player, Health{100, 100});
   * @endcode
   *
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
   * @code
   * // Direct construction
   * registry.emplace_component<Position>(entity, 100.0, 200.0, 1);
   *
   * // Equivalent to (but more efficient than):
   * registry.add_component(entity, Position(100.0, 200.0, 1));
   * @endcode
   *
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
   * @code
   * // Remove velocity to stop movement
   * registry.remove_component<Velocity>(entity);
   * // Entity still exists with other components
   * @endcode
   *
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
  void run_systems(EventManager&);

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
  void update_bindings(EventManager&);

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
        std::string comp = source_hook.substr(0, source_hook.find(':')) + "{"
            + std::to_string(entity) + "}";
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

    std::function<void()> deleter = [this, entity, source_hook]()
    {
      std::string hook_name = source_hook.substr(0, source_hook.find(':')) + "{"
          + std::to_string(entity) + "}";
      this->_hooked_components.erase(hook_name);
    };

    _bindings.emplace_back(entity,
                           ti,
                           field_name,
                           source_hook,
                           std::move(updater),
                           std::move(deleter),
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
   * @code
   * registry.init_scene_management();
   * registry.add_scene("menu", SceneState::MAIN);
   * registry.add_scene("hud", SceneState::ACTIVE);
   * registry.add_scene("gameplay", SceneState::DISABLED);
   * @endcode
   *
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
   * @code
   * // Switch from menu to gameplay
   * registry.remove_current_scene("menu");
   * registry.set_current_scene("gameplay");
   * @endcode
   *
   * @code
   * // Show pause menu over gameplay
   * // (both scenes active - systems process both)
   * registry.set_current_scene("gameplay");  // MAIN
   * registry.set_current_scene("pause_menu");  // ACTIVE overlay
   * @endcode
   *
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
   * @code
   * // User clicks "Start Game"
   * registry.remove_current_scene("main_menu");
   * registry.set_current_scene("gameplay");
   * @endcode
   *
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

  // ============================================================================
  // HOOK SYSTEM
  // ============================================================================

  // Hooks enable components to expose fields for runtime reflection and data
  // binding. This powers JSON configuration, network synchronization, and
  // dynamic component-to-component data flow.
  //
  // Hook Registration:
  // Components declare hookable fields using HOOKABLE() macro:
  //   HOOKABLE(Position, HOOK(x), HOOK(y))
  //
  // This creates a static hook_map() returning field accessors.
  //
  // Hook Usage:
  // 1. register_hook<Component>("name", entity) - Make component fields
  // accessible
  // 2. get_hooked_value<T>("name", "field") - Read field value by string name
  //
  // Hook Applications:
  // - JSON configuration: "%Player:maxSpeed" reads Player component's maxSpeed
  // - Dynamic bindings: "#Leader:pos" creates live binding to Leader's position
  // - Network sync: Serialize/deserialize specific fields by name
  // - Runtime introspection: Query component state without compile-time
  // knowledge
  //
  // Hook Naming:
  // The "name" parameter is a user-defined identifier (often entity name or
  // role). Multiple entities can have hooks with different names pointing to
  // same component type.
  //
  //   register_hook<Position>("player1", player1_entity)
  //   register_hook<Position>("player2", player2_entity)
  //   auto p1_x = get_hooked_value<float>("player1", "x")
  //   auto p2_x = get_hooked_value<float>("player2", "x")

  /**
   * @brief Register component hooks for runtime field access.
   *
   * Makes the specified entity's component fields accessible via string names.
   * The component type must be hookable (declared with HOOKABLE macro).
   *
   * After registration, get_hooked_value() can retrieve field values using
   * the hook name and field name. This enables JSON configuration to reference
   * runtime values with syntax like "%hookname:fieldname".
   *
   * The hook persists until the Registry is destroyed or the entity is deleted.
   * Registering the same name again overwrites the previous registration.
   *
   * @tparam T Component type (must satisfy hookable concept)
   * @param name Unique identifier for this hook (user-defined)
   * @param e Entity containing the component
   *
   * @note Component must have HOOKABLE() declaration
   * @note Hook name should be unique within the Registry
   * @note Entity must have the component when accessed
   *
   * @code
   * // Component with hookable fields
   * struct PlayerConfig {
   *   float max_speed = 100.0f;
   *   int max_health = 100;
   *
   *   HOOKABLE(PlayerConfig,
   *            HOOK(max_speed),
   *            HOOK(max_health))
   * };
   *
   * // Register hook for global config entity
   * Entity config_entity = registry.spawn_entity();
   * registry.add_component<PlayerConfig>(config_entity);
   * registry.register_hook<PlayerConfig>("PlayerConfig", config_entity);
   *
   * // Now JSON can reference these values
   * // {"speed": "%PlayerConfig:max_speed"}  // Uses hooked value
   * @endcode
   *
   * @code
   * // Register multiple player entities
   * Entity p1 = registry.spawn_entity();
   * Entity p2 = registry.spawn_entity();
   * registry.add_component<Position>(p1, 10.0f, 20.0f);
   * registry.add_component<Position>(p2, 50.0f, 60.0f);
   *
   * registry.register_hook<Position>("player1", p1);
   * registry.register_hook<Position>("player2", p2);
   *
   * // Access specific player positions
   * auto p1_x = registry.get_hooked_value<float>("player1", "x");
   * auto p2_x = registry.get_hooked_value<float>("player2", "x");
   * @endcode
   *
   * @code
   * // Register player as target for AI
   * Entity player = spawn_player(registry);
   * registry.register_hook<Position>("player", player);
   *
   * // Enemy JSON can reference player position
   * // {
   * //   "enemy": {
   * //     "components": {
   * //       "AI": {
   * //         "target": "#player:pos"  // Binds to player position
   * //       }
   * //     }
   * //   }
   * // }
   * @endcode
   *
   * @code
   * struct GameConfig {
   *   float gravity = 9.8f;
   *   float friction = 0.95f;
   *
   *   HOOKABLE(GameConfig, HOOK(gravity), HOOK(friction))
   * };
   *
   * // Create global config entity
   * Entity config = registry.spawn_entity();
   * registry.add_component<GameConfig>(config);
   * registry.register_hook<GameConfig>("GameConfig", config);
   *
   * // All entity configs can reference global settings
   * // {"friction": "%GameConfig:friction"}
   * @endcode
   *
   * @see get_hooked_value() to retrieve field values
   * @see HOOKABLE() macro to declare hookable components
   * @see register_binding() which uses hooks for dynamic data binding
   */
  template<hookable T>
  void register_hook(std::string name, Entity const& e)
  {
    this->_hooked_components.insert_or_assign(
        (name + "{" + std::to_string(e) + "}"),
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

  /**
   * @brief Retrieve a reference to a hooked component field.
   *
   * Returns a reference to a specific field of a hooked component. The
   * component must have been registered with register_hook(). Returns
   * std::nullopt if the hook name doesn't exist, the entity was deleted, or the
   * field name is invalid.
   *
   * The returned value is a reference_wrapper - changes to the original field
   * are reflected in the reference.
   *
   * @tparam T Field type (must match the actual field type)
   * @param comp Hook name (from register_hook())
   * @param value Field name (from HOOK() declaration)
   * @return Optional reference to the field, or std::nullopt if not found
   *
   * @note Returns std::nullopt if hook name is invalid
   * @note Returns std::nullopt if entity no longer has component
   * @note Returns std::nullopt if field name is invalid
   * @note Returned reference is valid until component is removed
   *
   * @code
   * // Setup
   * registry.register_hook<Position>("player", player_entity);
   *
   * // Read field
   * auto x_ref = registry.get_hooked_value<float>("player", "x");
   * if (x_ref) {
   *   float x = x_ref->get();  // Dereference reference_wrapper
   *   std::cout << "Player X: " << x << "\n";
   * }
   * @endcode
   *
   * @code
   * auto health_ref = registry.get_hooked_value<int>("player", "health");
   * if (health_ref) {
   *   health_ref->get() -= 10;  // Modify original component field
   * }
   * @endcode
   *
   * @code
   * template<typename T>
   * T get_config_value(Registry& r, const std::string& key) {
   *   // Config stored in "GameConfig" hook
   *   auto ref = r.get_hooked_value<T>("GameConfig", key);
   *   if (ref) {
   *     return ref->get();
   *   }
   *   throw std::runtime_error("Config key not found: " + key);
   * }
   *
   * float gravity = get_config_value<float>(registry, "gravity");
   * int max_enemies = get_config_value<int>(registry, "max_enemies");
   * @endcode
   *
   * @code
   * void update_enemy_ai(Registry& r, Entity enemy) {
   *   // Track player position through hook
   *   auto target_x = r.get_hooked_value<float>("player", "x");
   *   auto target_y = r.get_hooked_value<float>("player", "y");
   *
   *   if (target_x && target_y) {
   *     auto& enemy_pos = r.get_components<Position>()[enemy];
   *     if (enemy_pos) {
   *       // Move towards player
   *       float dx = target_x->get() - enemy_pos->x;
   *       float dy = target_y->get() - enemy_pos->y;
   *       // ... movement logic
   *     }
   *   }
   * }
   * @endcode
   *
   * @code
   * // This is how register_binding() uses get_hooked_value()
   * void sync_follower_position(Registry& r, Entity follower) {
   *   auto leader_x = r.get_hooked_value<float>("leader", "x");
   *   auto leader_y = r.get_hooked_value<float>("leader", "y");
   *
   *   if (leader_x && leader_y) {
   *     auto& follower_pos = r.get_components<Position>()[follower];
   *     if (follower_pos) {
   *       follower_pos->x = leader_x->get();
   *       follower_pos->y = leader_y->get();
   *     }
   *   }
   * }
   * @endcode
   *
   * @code
   * void debug_print_hook(Registry& r, const std::string& hook_name) {
   *   std::cout << "Hook '" << hook_name << "' fields:\n";
   *
   *   // Try common field names
   *   auto x = r.get_hooked_value<float>(hook_name, "x");
   *   if (x) std::cout << "  x: " << x->get() << "\n";
   *
   *   auto y = r.get_hooked_value<float>(hook_name, "y");
   *   if (y) std::cout << "  y: " << y->get() << "\n";
   *
   *   auto health = r.get_hooked_value<int>(hook_name, "health");
   *   if (health) std::cout << "  health: " << health->get() << "\n";
   * }
   * @endcode
   *
   * @see register_hook() to register component hooks
   * @see register_binding() which uses this for dynamic data binding
   * @see HOOKABLE() macro for declaring hookable fields
   */
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

  /**
   * @brief Register an entity template for reuse.
   *
   * Stores a JSON configuration as a named template. Templates are entity
   * prefabs - reusable entity definitions that can be instantiated multiple
   * times with EntityLoader::load_entity().
   *
   * Common use cases:
   * - Enemy types (basic_enemy, tank_enemy, fast_enemy)
   * - Powerups (health_pack, speed_boost, shield)
   * - UI elements (button, text_field, health_bar)
   * - Projectiles (bullet, missile, laser)
   *
   * @param name Unique template identifier
   * @param config JSON object defining the entity structure
   *
   * @note If template exists, it is overwritten
   * @note Template names should be unique
   * @note Templates are not validated until instantiated
   *
   * @code
   * JsonObject enemy_template = {
   *   {"components", {
   *     {"Position", {{"x", 0}, {"y", 0}}},
   *     {"Velocity", {{"speed", 50.0}}},
   *     {"Health", {{"max", 100}, {"current", 100}}},
   *     {"Sprite", {{"texture", "enemy.png"}}}
   *   }}
   * };
   *
   * registry.add_template("basic_enemy", enemy_template);
   *
   * // Later, instantiate multiple enemies
   * auto config1 = registry.get_template("basic_enemy");
   * config1["components"]["Position"]["x"] = 100;
   * EntityLoader(registry).load_entity(config1);
   *
   * auto config2 = registry.get_template("basic_enemy");
   * config2["components"]["Position"]["x"] = 200;
   * EntityLoader(registry).load_entity(config2);
   * @endcode
   *
   * @code
   * void load_templates(Registry& r, const std::string& file_path) {
   *   auto json = nlohmann::json::parse(std::ifstream(file_path));
   *
   *   for (const auto& [name, config] : json["templates"].items()) {
   *     r.add_template(name, config);
   *   }
   * }
   *
   * // templates.json:
   * // {
   * //   "templates": {
   * //     "player": { ... },
   * //     "enemy_basic": { ... },
   * //     "enemy_tank": { ... }
   * //   }
   * // }
   * @endcode
   *
   * @see get_template() to retrieve template
   * @see EntityLoader::load_entity() to instantiate template
   */
  void add_template(std::string const& name, JsonObject const& config);

  /**
   * @brief Retrieve a registered entity template.
   *
   * Returns a copy of the JSON configuration for the named template. The
   * returned JSON can be modified and passed to EntityLoader::load_entity()
   * to create an entity instance.
   *
   * @param name Template identifier (from add_template())
   * @return Copy of the template JSON configuration
   *
   * @note Returns empty/default JSON if template not found (prints error)
   * @note Returned JSON is a copy - modifications don't affect stored template
   * @note Use add_template() to update the stored template
   *
   * @code
   * // Get template and customize
   * auto enemy_config = registry.get_template("basic_enemy");
   * enemy_config["components"]["Position"]["x"] = 500;
   * enemy_config["components"]["Position"]["y"] = 300;
   *
   * // Create entity
   * EntityLoader loader(registry);
   * Entity enemy = loader.load_entity(enemy_config);
   * @endcode
   *
   * @code
   * bool has_template(Registry& r, const std::string& name) {
   *   auto config = r.get_template(name);
   *   return !config.empty();  // Empty if not found
   * }
   *
   * void load_entity_safe(Registry& r, const std::string& template_name) {
   *   if (has_template(r, template_name)) {
   *     auto config = r.get_template(template_name);
   *     EntityLoader(r).load_entity(config);
   *   } else {
   *     std::cerr << "Template not found: " << template_name << "\n";
   *   }
   * }
   * @endcode
   *
   * @see add_template() to register templates
   * @see EntityLoader::load_entity() to instantiate entities
   */
  JsonObject get_template(std::string const& name);

  bool is_in_current_cene(Entity e);

  // ============================================================================
  // NETWORK SUPPORT
  // ============================================================================
  //
  // Entity ID mapping and state serialization for networked multiplayer.
  //
  // The Problem:
  // In multiplayer games, entity IDs differ between client and server. The
  // server spawns entity 5, but on the client it becomes entity 12. Events
  // and components referencing entity IDs must be translated when transmitted.
  //
  // Entity Conversion:
  // convert_event_entity() and convert_comp_entity() remap entity IDs in
  // serialized data using a provided mapping. This ensures events like
  // "Player 5 shot Player 8" are correctly translated to client IDs.
  //
  // State Synchronization:
  // get_state() captures the entire ECS state as serialized components.
  // This enables full state snapshots for:
  // - New player connection (send world state)
  // - Save/load functionality
  // - Replay recording
  // - State diffing/delta compression
  //
  // String Keys:
  // get_event_key() and get_component_key() return the string identifier for
  // a type. This is used for network protocols where types are sent as strings
  // rather than compile-time type information.
  //
  // Network Flow:
  // 1. Server emits event with server entity IDs
  // 2. convert_event_entity() remaps IDs to client space
  // 3. Client emits event with client entity IDs
  // 4. Systems process with local IDs

  /**
   * @brief Convert entity IDs in a serialized event using provided
   * mapping.
   *
   * Remaps entity references in component data from one ID space to another.
   * Similar to convert_component_entity() but for events. Used when
   * synchronizing event state over network.
   *
   * The component type must implement change_entity(map) method that returns
   * a new event with remapped IDs.
   *
   * @param id event type string identifier
   * @param event Serialized event data (binary format)
   * @param map Entity ID mapping (old ID -> new ID)
   * @return Serialized event with remapped entity IDs
   *
   * @throws std::out_of_range If event type not registered
   *
   * @note event type must have change_entity() method
   */
  /**
   * @brief Convert entity IDs in a serialized component using provided
   * mapping.
   *
   * Remaps entity references in component data from one ID space to another.
   * Similar to convert_event_entity() but for components. Used when
   * synchronizing component state over network.
   *
   * The component type must implement change_entity(map) method that returns
   * a new component with remapped IDs.
   *
   * @param id Component type string identifier
   * @param comp Serialized component data (binary format)
   * @param map Entity ID mapping (old ID -> new ID)
   * @return Serialized component with remapped entity IDs
   *
   * @throws std::out_of_range If component type not registered
   *
   * @note Component type must have change_entity() method
   */
  ByteArray convert_comp_entity(std::string const& id,
                                ByteArray const& comp,
                                std::unordered_map<Entity, Entity> const& map);

  template<component Component>
  std::string get_component_key()
  {
    return this->_index_getter.at_first(typeid(Component));
  }

  /**
   * @brief Get the complete ECS state as serialized components.
   *
   * Captures a snapshot of all component data across all entities. Returns
   * a vector where each entry contains one component type's complete state:
   * the component type identifier and all (entity, component data) pairs.
   *
   * This is used for:
   * - Full state synchronization for new players
   * - Save/load game functionality
   * - Replay recording
   * - State diffing for delta compression
   *
   * The returned data is fully serialized and can be transmitted over network
   * or written to disk.
   *
   * @return Vector of ComponentState (one per component type)
   *
   * @note This is a complete snapshot - can be large for complex worlds
   * @note Each ComponentState contains component ID and entity-data pairs
   * @note Order is not guaranteed
   *
   * @code
   * void send_world_state_to_new_player(Registry& server_registry,
   *                                     NetworkClient& client,
   *                                     const EntityMapping& id_map) {
   *   // Get complete server state
   *   auto state = server_registry.get_state();
   *
   *   // Send each component type
   *   for (const auto& comp_state : state) {
   *     // comp_state.id = "Position", "Velocity", etc.
   *     // comp_state.comps = [(entity, bytes), (entity, bytes), ...]
   *
   *     for (const auto& [entity, data] : comp_state.comps) {
   *       // Remap entity IDs to client space
   *       ByteArray client_data = server_registry.convert_comp_entity(
   *         comp_state.id,
   *         data,
   *         id_map
   *       );
   *
   *       // Send to client
   *       client.send_component(comp_state.id, id_map.at(entity),
   * client_data);
   *     }
   *   }
   * }
   * @endcode
   *
   * @see ComponentState structure
   * @see convert_comp_entity() to remap entity IDs in state
   */
  std::vector<ComponentState> get_state();

private:
  struct Binding
  {
    Entity target_entity;  ///< Entity containing the target component
    std::type_index target_component;  ///< Type of the target component
    std::string target_field;  ///< Field name to update
    std::string source_hook;  ///< Hook string (e.g., "player:position")
    std::function<void()> updater;  ///< Copies value from hook to field
    std::function<void()> deleter;  ///< Cleans up binding
    std::function<ByteArray()>
        serializer;  ///< Serializes component for network sync

    Binding(Entity e,
            std::type_index ti,
            std::string tf,
            std::string sh,
            std::function<void()> u,
            std::function<void()> d,
            std::function<ByteArray()> s)
        : target_entity(e)
        , target_component(ti)
        , target_field(std::move(tf))
        , source_hook(std::move(sh))
        , updater(std::move(u))
        , deleter(std::move(d))
        , serializer(std::move(s))
    {
    }
  };

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
      _comp_entity_converters;

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
