#pragma once

#include <stdexcept>
#include <utility>

#include "NetworkShared.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/events/LoggerEvent.hpp"

template<component Component>
typename SparseArray<Component>::Ref init_component(Registry& r,
                                                    Registry::Entity to,
                                                    Component comp)
{
  try {
    r.emit<ComponentBuilder>(ComponentBuilder(
        to, r.get_component_key<Component>(), comp.to_bytes()));
  } catch (std::out_of_range const&) {
    r.emit<LogEvent>("init", LogLevel::ERROR, "unknow component");
  }
  return r.add_component<Component>(to, std::move(comp));
}

template<component Component, typename... Args>
typename SparseArray<Component>::Ref init_component(Registry& r,
                                                    Registry::Entity to,
                                                    Args... args)
{
  try {
    r.emit<ComponentBuilder>(ComponentBuilder(
        to, r.get_component_key<Component>(), Component(args...).to_bytes()));
  } catch (std::out_of_range const&) {
    r.emit<LogEvent>("init", LogLevel::ERROR, "unknow component");
  }
  return r.emplace_component<Component>(to, std::forward<Args>(args)...);
}

inline void init_component(Registry& r,
                           Registry::Entity to,
                           std::string const& id,
                           ByteArray const& comp)
{
  try {
    r.emit<ComponentBuilder>(ComponentBuilder(to, id, comp));
  } catch (std::out_of_range const&) {
    r.emit<LogEvent>("init", LogLevel::ERROR, "unknow component");
  }
  r.emplace_component(to, id, comp);
}
