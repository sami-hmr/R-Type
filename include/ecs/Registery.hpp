#pragma once

#include <any>
#include <cstddef>
#include <functional>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include "SparseArray.hpp"

class Registery
{
public:
  using entity = std::size_t;

  template<class Component>
  SparseArray<Component>& register_component()
  {
    this->_components.insert_or_assign(std::type_index(typeid(Component)),
                                       SparseArray<Component>());
    SparseArray<Component>& comp = this->get_components<Component>();
    this->_delete_functions.push_back([&comp](entity const& e)
                                      { comp.erase(e); });
    return comp;
  }

  template<class Component>
  SparseArray<Component>& get_components()
  {
    return std::any_cast<SparseArray<Component>&>(
        this->_components.at(std::type_index(typeid(Component))));
  }

  template<class Component>
  SparseArray<Component> const& get_components() const
  {
    return std::any_cast<SparseArray<Component>&>(
        this->_components.at(std::type_index(typeid(Component))));
  }

  entity spawn_entity()
  {
    entity to_return = 0;
    if (this->_dead_entites.empty()) {
      to_return = this->_max;
      this->_max += 1;
    } else {
      to_return = this->_dead_entites.front();
      this->_dead_entites.pop();
    }
    return to_return;
  }

  void kill_entity(entity const& e)
  {
    for (auto const& f : this->_delete_functions) {
      f(e);
    }
    this->_dead_entites.push(e);
  }

  template<typename Component>
  typename SparseArray<Component>::ref add_component(entity const& to,
                                                     Component&& c)
  {
    return this->get_components<Component>().insert_at(to, std::forward(c));
  }

  template<typename Component, typename... Params>
  typename SparseArray<Component>::ref emplace_component(entity const& to,
                                                         Params&... p)
  {
    return this->get_components<Component>().insert_at(to, p...);
  }

  template<typename Component>
  void remove_component(entity const& from)
  {
    this->get_components<Component>().erase(from);
  }

  template<class... Components, typename Function>
  void add_system(Function&& f)
  {
    this->_frequent_systems.push_back(
        [this, f]() { f(*this, this->get_components<Components>()...); });
  }

  template<class... Components, typename Function>
  void add_system(Function const& f)
  {
    this->_frequent_systems.push_back(
        [this, f]() { f(*this, this->get_components<Components>()...); });
  }

  void run_systems()
  {
    for (auto const& f : this->_frequent_systems) {
      f();
    }
  }

private:
  std::unordered_map<std::type_index, std::any> _components;
  std::vector<std::function<void()>> _frequent_systems;
  std::vector<std::function<void(entity const&)>> _delete_functions;
  std::queue<entity> _dead_entites;
  std::size_t _max = 0;
};
