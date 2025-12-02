#pragma once

#include <algorithm>
#include <any>
#include <chrono>
#include <cstddef>
#include <functional>
#include <queue>
#include <random>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Clock.hpp"
#include "SparseArray.hpp"
#include "TwoWayMap.hpp"
#include "ecs/Systems.hpp"
#include "plugin/Byte.hpp"

/**
 * @brief The Registery class is the core of the ECS (Entity-Component-System)
 * architecture.
 *
 * It manages entities, their associated components, and the systems that
 * operate on them.
 */
class Registery
{
public:
  /**
   * @brief Type alias for an entity identifier.
   *
   */
  using Entity = std::size_t;
  using HandlerId = std::size_t;

  /**
   * @brief Registers a bytable component type with a string identifier.
   *
   * @tparam Component The type of the component to register (must satisfy
   * bytable concept).
   * @param string_id The string identifier for this component type.
   * @return SparseArray<Component>& A reference to the sparse array of the
   * registered component type.
   */
  template<bytable Component>
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
    this->_index_getter.insert(ti, string_id);
    return comp;
  }

  /**
   * @brief Registers a component type in the registry (non-bytable overload).
   *
   * @tparam Component The type of the component to register.
   * @return SparseArray<Component>& A reference to the sparse array of the
   * registered component type.
   */
  template<class Component>
  SparseArray<Component>& register_component()
  {
    std::type_index ti(typeid(Component));

    this->_components.insert_or_assign(ti, SparseArray<Component>());
    SparseArray<Component>& comp = this->get_components<Component>();

    this->_delete_functions.insert_or_assign(
        ti, [&comp](Entity const& e) { comp.erase(e); });
    return comp;
  }

  /**
   * @brief Get the components object
   *
   * @tparam Component The type of the component to retrieve.
   * @return SparseArray<Component>& A reference to the sparse array of the
   * specified component type.
   */
  template<class Component>
  SparseArray<Component>& get_components()
  {
    return std::any_cast<SparseArray<Component>&>(
        this->_components.at(std::type_index(typeid(Component))));
  }

  /**
   * @brief Get the components object (const version)
   *
   * @tparam Component The type of the component to retrieve.
   * @return SparseArray<Component> const& A const reference to the sparse array
   * of the specified component type.
   */
  template<class Component>
  SparseArray<Component> const& get_components() const
  {
    return std::any_cast<const SparseArray<Component>&>(
        this->_components.at(std::type_index(typeid(Component))));
  }

  template<class Component>
  bool has_component(const Entity& e) const
  {
    SparseArray<Component> const& comp = this->get_components<Component>();
    return (e < comp.size() && comp[e].has_value());
  }

  Entity spawn_entity()
  {
    Entity to_return = 0;
    if (this->_dead_entities.empty()) {
      to_return = this->_max;
      this->_max += 1;
    } else {
      to_return = this->_dead_entities.front();
      this->_dead_entities.pop();
    }
    return to_return;
  }

  /**
   * @brief Kills an entity, marking it for deletion and allowing its ID to be
   * reused.
   *
   * @param e The entity to kill.
   */
  void kill_entity(Entity const& e) { _entities_to_kill.insert(e); }

  bool is_entity_dying(Entity const& e) const
  {
    return _entities_to_kill.find(e) != _entities_to_kill.end();
  }

  void process_entity_deletions()
  {
    for (auto const& e : _entities_to_kill) {
      for (auto const& [_, f] : this->_delete_functions) {
        f(e);
      }
      this->_dead_entities.push(e);
    }
    _entities_to_kill.clear();
  }

  /**
   * @brief Adds a component to an entity.
   *
   * @tparam Component The type of the component to add.
   * @param to The entity to which the component will be added.
   * @param c The component to add.
   * @return SparseArray<Component>::Ref A reference to the added component.
   */
  template<typename Component>
  typename SparseArray<Component>::Ref add_component(Entity const& to,
                                                     Component&& c)
  {
    return this->get_components<Component>().insert_at(
        to, std::forward<Component>(c));
  }

  /**
   * @brief Constructs and adds a component to an entity.
   *
   * @tparam Component The type of the component to add.
   * @tparam Params The types of the parameters to construct the component.
   * @param to The entity to which the component will be added.
   * @param p The parameters to construct the component.
   * @return SparseArray<Component>::Ref A reference to the added component.
   */
  template<typename Component, typename... Params>
  typename SparseArray<Component>::Ref emplace_component(Entity const& to,
                                                         Params&&... p)
  {
    return this->get_components<Component>().insert_at(
        to, std::forward<Params>(p)...);
  }

  /**
   * @brief Constructs and adds a component to an entity using byte array (for
   * bytable components).
   *
   * @param to The entity to which the component will be added.
   * @param string_id The string identifier of the component type.
   * @param bytes The byte array to construct the component from.
   */
  void emplace_component(Entity const& to,
                         std::string const& string_id,
                         ByteArray const& bytes)
  {
    this->_emplace_functions.at(this->_index_getter.at(string_id))(to, bytes);
  }

  /**
   * @brief Removes a component from an entity.
   *
   * @tparam Component: The type of the component to remove.
   * @param from The entity from which the component will be removed.
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

  /**
   * @brief Runs all registered systems.
   *
   */
  void run_systems()
  {
    this->clock().tick();

    for (auto const& f : this->_frequent_systems) {
      f();
    }
    process_entity_deletions();
  }

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

  template<typename EventType>
  void off_all()
  {
    std::type_index type_id(typeid(EventType));
    _event_handlers.erase(type_id);
  }

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

  Clock& clock() { return _clock; }

  const Clock& clock() const { return _clock; }

private:
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
  TwoWayMap<std::type_index, std::string> _index_getter;

  std::unordered_map<std::type_index, std::any> _event_handlers;
  std::vector<System<>> _frequent_systems;
  std::queue<Entity> _dead_entities;
  std::unordered_set<Entity> _entities_to_kill;
  Clock _clock;
  std::size_t _max = 0;
};
