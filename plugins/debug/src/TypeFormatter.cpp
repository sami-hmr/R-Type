#include <vector>

#include "TypeFormatter.hpp"

#include "libs/Color.hpp"
#include "libs/Rect.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/components/Collidable.hpp"

/**
 * @file TypeFormatter.cpp
 * @brief Implementation of type formatter initialization
 *
 * Registers formatters for all common types used in the codebase.
 */

void initialize_type_formatters()
{
  auto& registry = TypeFormatterRegistry::instance();

  // Primitive types - register for reference_wrapper versions
  registry.register_formatter<std::reference_wrapper<int>>(
      format_primitive<int>);
  registry.register_formatter<std::reference_wrapper<double>>(
      format_primitive<double>);
  registry.register_formatter<std::reference_wrapper<float>>(
      format_primitive<float>);
  registry.register_formatter<std::reference_wrapper<bool>>(
      format_primitive<bool>);
  registry.register_formatter<std::reference_wrapper<unsigned char>>(
      format_primitive<unsigned char>);
  registry.register_formatter<std::reference_wrapper<std::string>>(
      format_string);

  // Custom types with operator<< - register for reference_wrapper versions
  registry.register_formatter<std::reference_wrapper<Vector2D>>(
      [](std::any const& v)
      {
        auto ref = std::any_cast<std::reference_wrapper<Vector2D>>(v);
        std::ostringstream oss;
        oss << ref.get();
        return oss.str();
      });

  registry.register_formatter<std::reference_wrapper<Color>>(
      [](std::any const& v)
      {
        auto ref = std::any_cast<std::reference_wrapper<Color>>(v);
        std::ostringstream oss;
        oss << ref.get();
        return oss.str();
      });

  registry.register_formatter<std::reference_wrapper<Rect>>(
      [](std::any const& v)
      {
        auto ref = std::any_cast<std::reference_wrapper<Rect>>(v);
        std::ostringstream oss;
        oss << ref.get();
        return oss.str();
      });

  registry.register_formatter<Color>(
      [](std::any const& v)
      {
        try {
          auto ref = std::any_cast<std::reference_wrapper<const Color>>(v);
          std::ostringstream oss;
          oss << ref.get();
          return oss.str();
        } catch (std::bad_any_cast const&) {
          auto ref = std::any_cast<std::reference_wrapper<Color>>(v);
          std::ostringstream oss;
          oss << ref.get();
          return oss.str();
        }
      });

  registry.register_formatter<Rect>(
      [](std::any const& v)
      {
        try {
          auto ref = std::any_cast<std::reference_wrapper<const Rect>>(v);
          std::ostringstream oss;
          oss << ref.get();
          return oss.str();
        } catch (std::bad_any_cast const&) {
          auto ref = std::any_cast<std::reference_wrapper<Rect>>(v);
          std::ostringstream oss;
          oss << ref.get();
          return oss.str();
        }
      });

  registry.register_formatter<Color>(
      [](std::any const& v)
      {
        auto ref = std::any_cast<std::reference_wrapper<const Color>>(v);
        std::ostringstream oss;
        oss << ref.get();
        return oss.str();
      });

  registry.register_formatter<Rect>(
      [](std::any const& v)
      {
        auto ref = std::any_cast<std::reference_wrapper<const Rect>>(v);
        std::ostringstream oss;
        oss << ref.get();
        return oss.str();
      });

  // Enums - register for reference_wrapper versions
  registry.register_formatter<std::reference_wrapper<CollisionType>>(
      format_enum<CollisionType>);

  // Containers - register for reference_wrapper versions
  registry.register_formatter<std::reference_wrapper<std::vector<std::string>>>(
      format_string_container<std::vector<std::string>>);
}
