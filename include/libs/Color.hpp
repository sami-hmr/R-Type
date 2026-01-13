#pragma once

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

/**
 * @file Color.hpp
 * @brief RGBA color representation with serialization support
 */

/**
 * @struct Color
 * @brief RGBA color with 8-bit channels
 *
 * Represents colors using four unsigned char components (0-255):
 * - r: Red channel
 * - g: Green channel
 * - b: Blue channel
 * - a: Alpha channel (transparency, 255 = fully opaque)
 *
 * Supports JSON parsing with optional hook system for dynamic color binding.
 * Provides predefined color constants (WHITE, BLACK, RED, GREEN, BLUE,
 * TRANSPARENT).
 *
 * @see colorToByte(), parseColor()
 */
struct Color
{
  Color()
      : r(0)
      , g(0)
      , b(0)
      , a(255)
  {
  }  // NOLINT

  /**
   * @brief Constructs a color with explicit RGBA values
   */
  Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
      : r(r)
      , g(g)
      , b(b)
      , a(a)
  {
  }

  /**
   * @brief Constructs from JSON object
   * @param obj JSON object with r, g, b, and optional a fields
   * @note Supports hook system for dynamic color binding (see is_hook())
   * @note Missing or invalid values default to (0, 0, 0, 255)
   */
  Color(JsonObject const& obj)
  {
    try {
      if (!is_hook(obj, "r")) {
        this->r = static_cast<unsigned char>(std::get<int>(obj.at("r").value));
      }
      if (!is_hook(obj, "g")) {
        this->g = static_cast<unsigned char>(std::get<int>(obj.at("g").value));
      }
      if (!is_hook(obj, "b")) {
        this->b = static_cast<unsigned char>(std::get<int>(obj.at("b").value));
      }
      if (obj.contains("a") && !is_hook(obj, "a")) {
        this->a = static_cast<unsigned char>(std::get<int>(obj.at("a").value));
      }
    } catch (std::bad_variant_access const&) {
      std::cerr << "Error parsing Color: unexpected value type\n";
      this->r = 0;
      this->g = 0;
      this->b = 0;
      this->a = 255;
    } catch (std::out_of_range const&) {
      std::cerr << "Error parsing Color: missing r, g, or b in JsonObject\n";
      this->r = 0;
      this->g = 0;
      this->b = 0;
      this->a = 255;
    }
  }

  Color(JsonVariant const& variant)
      : Color(std::get<JsonObject>(variant))
  {
  }

  unsigned char r;  ///< Red channel (0-255)
  unsigned char g;  ///< Green channel (0-255)
  unsigned char b;  ///< Blue channel (0-255)
  unsigned char a;  ///< Alpha channel (0=transparent, 255=opaque)
};

inline bool operator==(const Color& lhs, const Color& rhs)
{
  return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

/**
 * @brief Parses a Color from binary data
 * @return Parser that deserializes four bytes (r, g, b, a)
 */
inline Parser<Color> parseColor()  // NOLINT
{
  return apply(
      [](unsigned char r, unsigned char g, unsigned char b, unsigned char a)
      { return Color {r, g, b, a}; },
      parseByte<unsigned char>(),
      parseByte<unsigned char>(),
      parseByte<unsigned char>(),
      parseByte<unsigned char>());
}

/**
 * @brief Serializes a Color to binary data
 * @param c Color to serialize
 * @return ByteArray containing r, g, b, a bytes
 */
inline ByteArray colorToByte(const Color& c)  // NOLINT
{
  return byte_array_join(type_to_byte<unsigned char>(c.r),
                         type_to_byte<unsigned char>(c.g),
                         type_to_byte<unsigned char>(c.b),
                         type_to_byte<unsigned char>(c.a));
}

static const Color WHITE = Color(255, 255, 255, 255);
static const Color BLACK = Color(0, 0, 0, 255);
static const Color RED = Color(255, 0, 0, 255);
static const Color GREEN = Color(0, 255, 0, 255);
static const Color BLUE = Color(0, 0, 255, 255);
static const Color YELLOW = Color(255, 255, 0, 255);
static const Color TRANSPARENT = Color(0, 0, 0, 0);
