#pragma once

#include <stdexcept>
#include "NetworkShared.hpp"
#include "ecs/Registry.hpp"
#include "plugin/events/LoggerEvent.hpp"

template <component Component>
typename SparseArray<Component>::Ref init_component(Registry &r, Registry::Entity to, Component comp) {
    try {
    r.emit<ComponentBuilder>(ComponentBuilder(
        to,
        r.get_component_key<Component>(),
        comp.to_bytes()));
    } catch (std::out_of_range const &) {
        r.emit<LogEvent>("init", LogLevel::ERROR, "unknow component");
    }
    return r.add_component<Component>(to, std::move(comp));
}
