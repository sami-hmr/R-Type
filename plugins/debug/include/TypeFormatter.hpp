#pragma once

#include <any>
#include <concepts>
#include <cstdint>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

/**
 * @file TypeFormatter.hpp
 * @brief Generic type-to-string converter for debug output
 *
 * Provides a type registry system that can format std::any values containing
 * std::reference_wrapper<T> to human-readable strings. Supports primitives,
 * custom types with operator<<, enums, containers, and provides fallback for
 * unknown types.
 */

/**
 * @concept Streamable
 * @brief Types that support stream output operator
 */
template<typename T>
concept Streamable = requires(std::ostream& os, const T& t) {
  { os << t } -> std::same_as<std::ostream&>;
};

/**
 * @concept Container
 * @brief Types that have begin/end iterators
 */
template<typename T>
concept Container = requires(T t) {
  { t.begin() };
  { t.end() };
  typename T::value_type;
};

/**
 * @concept StringContainer
 * @brief Containers specifically holding strings
 */
template<typename T>
concept StringContainer =
    Container<T> && std::is_same_v<typename T::value_type, std::string>;

/**
 * @class TypeFormatterRegistry
 * @brief Singleton registry for type-to-string formatters
 *
 * Maintains a map of type_index to formatter functions. Each formatter knows
 * how to extract a value from std::any (containing reference_wrapper<T>) and
 * convert it to a string.
 */
class TypeFormatterRegistry
{
public:
  using FormatterFunc = std::function<std::string(std::any const&)>;

  /**
   * @brief Get singleton instance
   */
  static TypeFormatterRegistry& instance()
  {
    static TypeFormatterRegistry registry;
    return registry;
  }

  /**
   * @brief Register a formatter for a specific type
   * @tparam T The type to register a formatter for
   * @param formatter Function that converts std::any to string
   */
  template<typename T>
  void register_formatter(FormatterFunc formatter)
  {
    _formatters[std::type_index(typeid(T))] = std::move(formatter);
  }

  /**
   * @brief Format an std::any value to string
   * @param value The value to format (should contain reference_wrapper<T>)
   * @param type_idx The type index of T
   * @return Formatted string, or "<unknown type>" if no formatter registered
   */
  std::string format(std::any const& value,
                     std::type_index const& type_idx) const
  {
    auto it = _formatters.find(type_idx);
    if (it != _formatters.end()) {
      try {
        return it->second(value);
      } catch (std::bad_any_cast const&) {
        return "<bad_any_cast>";
      } catch (std::exception const& e) {
        return std::string("<error: ") + e.what() + ">";
      }
    }
    return std::string("<unknown type: ") + type_idx.name() + ">";
  }

  /**
   * @brief Check if a formatter is registered for a type
   */
  bool has_formatter(std::type_index const& type_idx) const
  {
    return _formatters.contains(type_idx);
  }

private:
  TypeFormatterRegistry() = default;
  std::unordered_map<std::type_index, FormatterFunc> _formatters;
};

/**
 * @brief Format primitive types (int, double, bool, string)
 */
template<typename T>
  requires std::is_arithmetic_v<T>
std::string format_primitive(std::any const& value)
{
  auto ref = std::any_cast<std::reference_wrapper<T>>(value);
  if constexpr (std::is_same_v<T, bool>) {
    return ref.get() ? "true" : "false";
  } else if constexpr (std::is_integral_v<T>) {
    return std::to_string(static_cast<int64_t>(ref.get()));
  } else {
    return std::to_string(ref.get());
  }
}

/**
 * @brief Format std::string
 */
inline std::string format_string(std::any const& value)
{
  auto ref = std::any_cast<std::reference_wrapper<std::string>>(value);
  return "\"" + ref.get() + "\"";
}

/**
 * @brief Format types with operator<<
 */
template<Streamable T>
std::string format_streamable(std::any const& value)
{
  auto ref = std::any_cast<std::reference_wrapper<T>>(value);
  std::ostringstream oss;
  oss << ref.get();
  return oss.str();
}

/**
 * @brief Format enums as their underlying integer value
 */
template<typename T>
  requires std::is_enum_v<T>
std::string format_enum(std::any const& value)
{
  auto ref = std::any_cast<std::reference_wrapper<T>>(value);
  return std::to_string(static_cast<std::underlying_type_t<T>>(ref.get()));
}

/**
 * @brief Format string containers like vector<string>
 */
template<StringContainer T>
std::string format_string_container(std::any const& value)
{
  auto ref = std::any_cast<std::reference_wrapper<T>>(value);
  std::ostringstream oss;
  oss << "[";
  bool first = true;
  for (auto const& elem : ref.get()) {
    if (!first) {
      oss << ", ";
    }
    oss << "\"" << elem << "\"";
    first = false;
  }
  oss << "]";
  return oss.str();
}

/**
 * @brief Format generic containers with streamable elements
 */
template<Container T>
  requires Streamable<typename T::value_type>
    && (!std::is_same_v<typename T::value_type, std::string>)
std::string format_container(std::any const& value)
{
  auto ref = std::any_cast<std::reference_wrapper<T>>(value);
  std::ostringstream oss;
  oss << "[";
  bool first = true;
  for (auto const& elem : ref.get()) {
    if (!first) {
      oss << ", ";
    }
    oss << elem;
    first = false;
  }
  oss << "]";
  return oss.str();
}

/**
 * @brief Initialize all common type formatters
 *
 * This function should be called once to register formatters for all
 * commonly used types in the codebase.
 */
void initialize_type_formatters();
