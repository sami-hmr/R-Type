#pragma once

#include <any>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"

#define GAUTHIER_SOIT_PAS_DEBILE_STP(key, var) \
  {#key, \
   [](Component &self) -> std::any \
   { \
     return std::reference_wrapper(self.var); \
   }}

#define HOOK(var) GAUTHIER_SOIT_PAS_DEBILE_STP(var, var)

#define HOOKABLE(type, ...) \
  using Component = type; \
  static const auto& hook_map() { \
    static const std::unordered_map<std::string, std::function<std::any(type &)>> map{ \
        __VA_ARGS__\
      };\
      return map; \
  } \

template<typename T>
std::optional<std::reference_wrapper<const T>> get_ref(Registery& r,
                                                 JsonObject const &object,
                                                 std::string const& key)
{
  try {
    std::string hook = std::get<std::string>(object.at(key).value);
    try {
        if (hook.starts_with('#')) {
            std::string striped = hook.substr(1);
            std::string comp = striped.substr(0, striped.find(':'));
            std::string value = striped.substr(striped.find(':') + 1);
            return r.get_hooked_value<T>(comp, value);
        }
    } catch (std::bad_any_cast const &) {
        std::cerr << std::format(R"(Error geting hooked value "{}": Invalid type\n)", hook);
        return std::nullopt;
    } catch (std::out_of_range const &) {
        std::cerr << std::format(R"(Error geting hooked value "{}": Invalid hook\n)", hook);
        return std::nullopt;
    }
  } catch (std::bad_variant_access const&) { // NOLINT intentional fallthrought
  }
  try {
      return std::reference_wrapper<const T>(std::get<T>(object.at(key).value));
  } catch (...) {
      return std::nullopt;
  }
}

template<typename T>
std::optional<T> get_value(Registery& r,
                           JsonObject const &object,
                           std::string const& key)
{
  auto tmp = get_ref<T>(r, object, key);
  if (tmp.has_value()) {
    return tmp.value().get();
  }
  return std::nullopt;
}
