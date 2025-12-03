#pragma once

#include <concepts>
#include <functional>
#include <optional>
#include <string>
#include <variant>
#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"

#define HOOK(var) {#var, [this]() -> std::any {return std::reference_wrapper(this->var);}}

#define HOOKABLE(...) \
    std::unordered_map<std::string, std::function<std::any ()>> hook_map{ \
        __VA_ARGS__\
    }; \


template <typename T>
std::optional<std::reference_wrapper<T>> get_ref(Registery &r, JsonObject &object, std::string const &key) {
    try {
        std::string hook = std::get<std::string>(object.at(key).value);
        if (hook.starts_with('#')) {
            hook = hook.substr(1);
            std::string comp = hook.substr(0, hook.find(':'));
            std::string value = hook.substr(hook.find(':') + 1);
            return r.get_hooked_value<T>(comp, value);
        }
    } catch (std::bad_variant_access const &) {} //NOLINT intentional fallthrought
    return std::reference_wrapper<T>(std::get<T>(object.at(key).value));

}

template <typename T>
std::optional<T> get_value(Registery &r, JsonObject &object, std::string const &key) {
    auto tmp = get_ref<T>(r, object, key);
    if (tmp.has_value()) {
        return tmp.value().get();
    }
    return std::nullopt;
}
