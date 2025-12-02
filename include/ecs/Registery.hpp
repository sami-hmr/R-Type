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
#include <utility>
#include <vector>

#include "SparseArray.hpp"
#include "Systems.hpp"

/**
 * @brief The Registery class is the core of the ECS (Entity-Component-System) architecture.
 * 
 * It manages entities, their associated components, and the systems that operate on them.
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
   * @brief Registers a component type in the registry.
   * 
   * @tparam Component The type of the component to register.
   * @return SparseArray<Component>& A reference to the sparse array of the registered component type.
   */
  template<class Component>
  SparseArray<Component>& register_component()
  {
    this->_components.insert_or_assign(std::type_index(typeid(Component)),
                                       SparseArray<Component>());
    SparseArray<Component>& comp = this->get_components<Component>();
    this->_delete_functions.push_back([&comp](Entity const& e)
                                      { comp.erase(e); });
    return comp;
  }

  /**
   * @brief Get the components object
   * 
   * @tparam Component The type of the component to retrieve.
   * @return SparseArray<Component>& A reference to the sparse array of the specified component type.
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
   * @return SparseArray<Component> const& A const reference to the sparse array of the specified component type.
   */
  template<class Component>
  SparseArray<Component> const& get_components() const
  {
    return std::any_cast<SparseArray<Component>&>(
        this->_components.at(std::type_index(typeid(Component))));
  }

  /**
   * @brief Spawns a new entity.
   * If there are any previously killed entities, it reuses their IDs.
   *
   * @return Entity: The ID of the spawned entity.
   */
  Entity spawn_entity()
  {
    Entity to_return = 0;
    if (this->_dead_entites.empty()) {
      to_return = this->_max;
      this->_max += 1;
    } else {
      to_return = this->_dead_entites.front();
      this->_dead_entites.pop();
    }
    return to_return;
  }

  /**
   * @brief Kills an entity, marking it for deletion and allowing its ID to be reused.
   * 
   * @param e The entity to kill.
   */
  void kill_entity(Entity const& e)
  {
    for (auto const& f : this->_delete_functions) {
      f(e);
    }
    this->_dead_entites.push(e);
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
    for (auto const& f : this->_frequent_systems) {
      f();
    }
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

private:
  static HandlerId generate_uuid()
  {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<HandlerId> dis;
    return dis(gen);
  }

  std::unordered_map<std::type_index, std::any> _components;
  std::unordered_map<std::type_index, std::any> _event_handlers;
  std::vector<System<>> _frequent_systems;
  std::vector<std::function<void(Entity const&)>> _delete_functions;
  std::queue<Entity> _dead_entites;
  std::size_t _max = 0;
};
