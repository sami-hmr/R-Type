#pragma once

#include <any>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"


template<typename T>
struct is_in_json_variant
    : std::disjunction<std::is_same<T, int>,
                       std::is_same<T, double>,
                       std::is_same<T, std::string>,
                       std::is_same<T, bool>,
                       std::is_same<T, JsonObject>,
                       std::is_same<T, JsonArray>>
{
};

template<typename T>
inline constexpr bool is_in_json_variant_v = is_in_json_variant<T>::value;

template<typename T>
std::optional<std::reference_wrapper<const T>> get_ref(Registry& r,
                                                       JsonObject const& object,
                                                       std::string const& key)
{
  auto it = object.find(key);
  if (it == object.end()) {
    return std::nullopt;
  }

  auto const& value = it->second.value;

  try {
    std::string hook = std::get<std::string>(value);
    try {
      if (hook.starts_with('#')) {
        std::string striped = hook.substr(1);
        std::string comp = striped.substr(0, striped.find(':'));
        std::string value = striped.substr(striped.find(':') + 1);
        return r.get_hooked_value<T>(comp, value);
      }
    } catch (std::bad_any_cast const&) {
      std::cerr << std::format(
          R"(Error geting hooked value "{}": Invalid type\n)", hook);
      return std::nullopt;
    } catch (std::out_of_range const&) {
      std::cerr << std::format(
          R"(Error geting hooked value "{}": Invalid hook\n)", hook);
      return std::nullopt;
    }
  } catch (std::bad_variant_access const&) {  // NOLINT intentional fallthrought
  }

  if constexpr (is_in_json_variant_v<T>) {
    try {
      return std::reference_wrapper<const T>(std::get<T>(value));
    } catch (...) {
      return std::nullopt;
    }
  }

  return std::nullopt;
}

/*
 *@brief Gets the value at a given key from a JsonObject for a component ALSO
 * register the component for a **static** hook (usefull for when entity isn't
 * accessible)
 */
template<typename T, typename... Args>
std::optional<T> get_value_copy(Registry& r,
                                JsonObject const& object,
                                std::string const& key,
                                Args&&... args)
{
  auto tmp = get_ref<T>(r, object, key);
  if (tmp.has_value()) {
    return tmp.value().get();
  }

  if constexpr (std::is_constructible_v<T, JsonObject, Args...>) {
    try {
      const JsonObject& obj = std::get<JsonObject>(object.at(key).value);
      return T(obj, std::forward<Args>(args)...);
    } catch (...) {
    }
  }

  return std::nullopt;
}

/*
 *@brief Gets the value at a given key from a JsonObject for a component ALSO
 * register the component for a **dynamic** hook ALWAYS prefer to get_value_copy
 * to avoid ownsership issues
 */
template<typename ComponentType, typename T, typename... Args>
std::optional<T> get_value(Registry& r,
                           JsonObject const& object,
                           Registry::Entity entity,
                           std::string const& field_name,
                           Args&&... args)
{
  try {
    std::string value_str = std::get<std::string>(object.at(field_name).value);
    if (value_str.starts_with('#')) {
      std::string stripped = value_str.substr(1);
      r.template register_binding<ComponentType, T>(
          entity, field_name, stripped);
    }
  } catch (std::bad_variant_access const&) {  // NOLINT intentional fallthrough
  }

  return get_value_copy<T>(r, object, field_name, std::forward<Args>(args)...);
}

inline bool is_hook(JsonObject const& object, std::string const& key)
{
  try {
    std::string value = std::get<std::string>(object.at(key).value);
    return value.starts_with('#');
  } catch (...) {
    return false;
  }
}
