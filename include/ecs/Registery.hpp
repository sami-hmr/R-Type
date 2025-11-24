#pragma once

#include <any>
#include <cstddef>
#include <functional>
#include <iostream>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "SparseArray.hpp"

class Registery
{
public:
  using Entity = std::size_t;

  template<class Component>
  SparseArray<Component>& registerComponent()
  {
    this->components_.insert_or_assign(std::type_index(typeid(Component)),
                                       SparseArray<Component>());
    SparseArray<Component>& comp = this->getComponents<Component>();
    this->delete_functions_.push_back([&comp](Entity const& e)
                                      { comp.erase(e); });
    return comp;
  }

  template<class Component>
  SparseArray<Component>& getComponents()
  {
    return std::any_cast<SparseArray<Component>&>(
        this->components_.at(std::type_index(typeid(Component))));
  }

  template<class Component>
  SparseArray<Component> const& getComponents() const
  {
    return std::any_cast<SparseArray<Component>&>(
        this->components_.at(std::type_index(typeid(Component))));
  }

  Entity spawn_entity()
  {
    Entity toReturn;
    if (this->deadEntites_.empty()) {
      toReturn = this->max;
      this->max += 1;
    } else {
      toReturn = this->deadEntites_.front();
      this->deadEntites_.pop();
    }
    return toReturn;
  }

  void kill_entity(Entity const& e)
  {
    for (auto const& f : this->delete_functions_) {
      f(e);
    }
    this->deadEntites_.push(e);
  }

  template<typename Component>
  typename SparseArray<Component>::ref add_component(Entity const& to,
                                                     Component&& c)
  {
    return this->getComponents<Component>().insert_at(to, c);
  }

  template<typename Component, typename... Params>
  typename SparseArray<Component>::ref emplace_component(Entity const& to,
                                                         Params&&... p)
  {
    return this->getComponents<Component>().insert_at(to, p...);
  }

  template<typename Component>
  void removeComponent(Entity const& from)
  {
    this->getComponents<Component>().erase(from);
  }

  template<class... Components, typename Function>
  void addSystem(Function&& f)
  {
    this->frequent_systems_.push_back(
        [this, f]() { f(*this, this->getComponents<Components>()...); });
  }

  void runSystems()
  {
    for (auto const& f : this->frequent_systems_) {
      f();
    }
  }

private:
  std::unordered_map<std::type_index, std::any> components_;
  std::vector<std::function<void()>> frequent_systems_;
  std::vector<std::function<void(Entity const&)>> delete_functions_;
  std::queue<Entity> deadEntites_;
  std::size_t max = 0;
};
