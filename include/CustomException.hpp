/*
** EPITECH PROJECT, 2025
** raytracer
** File description:
** customException base class
*/

#ifndef CUSTOM_EXCEPTION_HPP
#define CUSTOM_EXCEPTION_HPP

#include <exception>
#include <format>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

class CustomException : public std::exception
{
public:
  explicit CustomException(std::string const& message)
      : _message(message)
  {
  }

  explicit CustomException(std::string&& message)
      : _message(std::move(message))
  {
  }

  const char* what() const noexcept override { return _message.c_str(); }

  /**
   * Retrieve a specific context value by key.
   *
   * @param key The context key to retrieve
   * @return Optional containing the value if found, empty otherwise
   */
  std::optional<std::string> get_context(std::string const& key) const
  {
    auto it = _context.find(key);
    return it != _context.end() ? std::optional(it->second) : std::nullopt;
  }

  /**
   * Get all context data.
   *
   * @return Map of all context key-value pairs
   */
  std::unordered_map<std::string, std::string> const& get_all_context() const
  {
    return _context;
  }

  /**
   * Format all context data as a human-readable string.
   * Format: "key1=value1, key2=value2, ..."
   *
   * @return Formatted context string, or empty string if no context
   */
  std::string format_context() const
  {
    if (_context.empty()) {
      return "";
    }

    std::ostringstream oss;
    oss << "Context: ";
    bool first = true;
    for (auto const& [key, value] : _context) {
      if (!first) {
        oss << ", ";
      }
      oss << key << "=" << value;
      first = false;
    }
    return oss.str();
  }

private:
  std::string _message;

protected:
  std::unordered_map<std::string, std::string> _context;
};

#define CUSTOM_EXCEPTION(name) \
  class name : public CustomException \
  { \
  public: \
    explicit name(const std::string& message) \
        : CustomException(std::format("From: {}; Error: {}", #name, message)) \
    { \
    } \
    name with_context(std::string const& key, std::string const& value) \
    { \
      _context.insert_or_assign(key, value); \
      return *this; \
    } \
  };

#endif
