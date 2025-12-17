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

template<typename T>
concept component = bytable<T> && entity_convertible<T>;

template<typename T>
concept event = bytable<T> && entity_convertible<T> && json_buildable<T>;

/**
 * @brief The Registry class is the core of the ECS (Entity-Component-System)
 * architecture.
 *
 * It manages entities, their associated components, and the systems that
 * operate on them.
 */
class Registry
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

  Entity spawn_entity();

  /**
   * @brief Kills an entity, marking it for deletion and allowing its ID to be
   * reused.
   *
   * @param e The entity to kill.
   */
  void kill_entity(Entity const& e);

  bool is_entity_dying(Entity const& e) const;

  void process_entity_deletions();

  /**
   * @brief Adds a component to an entity.
   *
   * @tparam Component The type of the component to add.
   * @param to The entity to which the component will be added.
   * @param c The component to add.
   * @return SparseArray<Component>::Ref A reference to the added component.
   */

  template<component Component>
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
                         ByteArray const& bytes);

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
  void run_systems();

  /**
   * @brief Updates all variable bindings from their hooked sources.
   *
   */
  void update_bindings();

  /**
   * @brief Registers a binding that syncs a variable to a hooked source every
   * tick.
   *
   * @tparam ComponentType The type of component containing the field.
   * @tparam T The type of the field being bound.
   * @param entity The entity containing the component.
   * @param field_name The name of the field to bind.
   * @param source_hook The hook string (e.g., "player:position").
   */
  template<typename ComponentType, typename T>
  void register_binding(Entity entity,
                        std::string const& field_name,
                        std::string const& source_hook)
  {
    std::type_index ti(typeid(ComponentType));

    auto updater = [this, entity, field_name, source_hook]()
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

    _bindings.emplace_back(
        entity, ti, field_name, source_hook, std::move(updater));
  }

  /**
   * @brief Clears all registered bindings.
   *
   */
  void clear_bindings();

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

  void emit(std::string const& name, JsonObject const& args);

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

  void emit(std::string const& name, ByteArray const& data);

  void add_scene(std::string const& scene_name, SceneState state);

  void init_scene_management();

  void setup_scene_systems();

  void set_current_scene(std::string const& scene_name);

  void remove_current_scene(std::string const& scene_name);

  void remove_all_scenes();

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

    Binding(Entity e,
            std::type_index ti,
            std::string tf,
            std::string sh,
            std::function<void()> u)
        : target_entity(e)
        , target_component(ti)
        , target_field(std::move(tf))
        , source_hook(std::move(sh))
        , updater(std::move(u))
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
