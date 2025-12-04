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
   [](Component& self) -> std::any \
   { return std::reference_wrapper(self.var); }}

#define HOOK(var) GAUTHIER_SOIT_PAS_DEBILE_STP(var, var)

#define HOOKABLE(type, ...) \
  using Component = type; \
  static const auto& hook_map() \
  { \
    static const std::unordered_map<std::string, \
                                    std::function<std::any(type&)>> \
        map {__VA_ARGS__}; \
    return map; \
  }

template<typename T>
std::optional<std::reference_wrapper<const T>> get_ref(Registery& r,
                                                       JsonObject const& object,
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
  try {
    return std::reference_wrapper<const T>(std::get<T>(object.at(key).value));
  } catch (...) {
    return std::nullopt;
  }
}

template<typename T>
std::optional<T> get_value(Registery& r,
                           JsonObject const& object,
                           std::string const& key)
{
  auto tmp = get_ref<T>(r, object, key);
  if (tmp.has_value()) {
    return tmp.value().get();
  }
  return std::nullopt;
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

template<typename T>
struct ValueRefPair
{
  T value;
  std::optional<std::reference_wrapper<const T>> ref;
};

template<typename T>
std::optional<ValueRefPair<T>> get_value_or_ref(Registery& r,
                                                JsonObject const& object,
                                                std::string const& key)
{
  if (is_hook(object, key)) {
    auto ref = get_ref<T>(r, object, key);
    if (!ref.has_value()) {
      return std::nullopt;
    }
    return ValueRefPair<T> {T {}, ref};
  }
  auto val = get_value<T>(r, object, key);
  if (!val.has_value()) {
    return std::nullopt;
  }
  return ValueRefPair<T> {val.value(), std::nullopt};
}

template<typename T>
class HookRef
{
public:
  HookRef()
      : _value {}
      , _ref(std::nullopt)
  {
  }

  HookRef(T value)
      : _value(std::move(value))
      , _ref(std::nullopt)
  {
  }

  HookRef(T value, std::optional<std::reference_wrapper<const T>> ref)
      : _value(std::move(value))
      , _ref(ref)
  {
  }

  const T& get() const
  {
    return _ref.has_value() ? _ref.value().get() : _value;
  }

  operator const T&() const { return get(); }

  void set(T value)
  {
    _value = std::move(value);
    _ref = std::nullopt;
  }

  void set_ref(std::optional<std::reference_wrapper<const T>> ref)
  {
    _ref = ref;
  }

  bool is_ref() const { return _ref.has_value(); }

private:
  T _value;
  std::optional<std::reference_wrapper<const T>> _ref;
};

template<typename T>
HookRef<T> get_hook_ref(Registery& r,
                        JsonObject const& object,
                        std::string const& key)
{
  auto pair = get_value_or_ref<T>(r, object, key);
  if (!pair.has_value()) {
    return HookRef<T> {};
  }
  return HookRef<T>(pair->value, pair->ref);
}
