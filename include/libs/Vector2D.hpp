/*
** EPITECH PROJECT, 2025
** Bootstrap
** File description:
** Vector2D
*/

#ifndef VECTOR2D_HPP_
#define VECTOR2D_HPP_

#include <cmath>
#include <iostream>
#include <ostream>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "Parser.hpp"
#include "plugin/Byte.hpp"

/**
 * @file Vector2D.hpp
 * @brief 2D vector mathematics for position, velocity, and direction
 * calculations
 */

/**
 * @class Vector2D
 * @brief Two-dimensional vector with mathematical operations
 *
 * Provides a comprehensive 2D vector implementation with:
 * - Arithmetic operations (addition, subtraction, multiplication, division)
 * - Vector mathematics (length, normalization, dot product, distance)
 * - JSON/ByteArray serialization for configuration and networking
 * - Comparison operators for sorting and equality checks
 *
 * Widely used throughout the engine for positions, velocities, directions,
 * scales, and any 2D spatial data.
 *
 * @note Component-wise comparison operators (<, >, <=, >=) check both x and y
 * @see Position, Velocity components
 */
class Vector2D
{
public:
  Vector2D() = default;

  /**
   * @brief Constructs a vector with x and y coordinates
   */
  Vector2D(double x, double y)
      : x(x)
      , y(y) {};

  /**
   * @brief Constructs from JSON variant with custom field names
   * @param variant JSON variant containing an object with x/y fields
   * @param x Field name for x coordinate (default: "x")
   * @param y Field name for y coordinate (default: "y")
   */
  Vector2D(JsonVariant const& variant,
           std::string const& x = "x",
           std::string const& y = "y");

  /**
   * @brief Constructs from JSON object with custom field names
   * @param obj JSON object containing x/y fields
   * @param x Field name for x coordinate (default: "x")
   * @param y Field name for y coordinate (default: "y")
   */
  Vector2D(JsonObject const& obj,
           std::string const& x = "x",
           std::string const& y = "y");
  ~Vector2D() = default;

  Vector2D(const Vector2D& other) = default;
  Vector2D& operator=(const Vector2D& other) = default;
  Vector2D(Vector2D&& other) noexcept = default;
  Vector2D& operator=(Vector2D&& other) noexcept = default;

  Vector2D operator+(const Vector2D& other) const
  {
    return {this->x + other.x, this->y + other.y};
  };

  Vector2D& operator+=(const Vector2D& other)
  {
    this->x += other.x;
    this->y += other.y;
    return *this;
  };

  Vector2D operator-(const Vector2D& other) const
  {
    return {this->x - other.x, this->y - other.y};
  };

  Vector2D& operator-=(const Vector2D& other)
  {
    this->x -= other.x;
    this->y -= other.y;
    return *this;
  };

  Vector2D operator*(double scalar) const
  {
    return {this->x * scalar, this->y * scalar};
  };

  Vector2D operator*(Vector2D other) const
  {
    return {this->x * other.x, this->y * other.y};
  }

  Vector2D& operator*=(Vector2D other)
  {
    this->x *= other.x;
    this->y *= other.y;
    return *this;
  }

  Vector2D& operator*=(double scalar)
  {
    this->x *= scalar;
    this->y *= scalar;
    return *this;
  };

  Vector2D operator/(double scalar) const
  {
    return {this->x / scalar, this->y / scalar};
  };

  Vector2D& operator/=(double scalar)
  {
    this->x /= scalar;
    this->y /= scalar;
    return *this;
  };

  bool operator==(const Vector2D& other) const
  {
    return this->x == other.x && this->y == other.y;
  };

  bool operator!=(const Vector2D& other) const { return !(*this == other); };

  bool operator<=(const Vector2D& other) const
  {
    return (this->x <= other.x && this->y <= other.y);
  };

  bool operator>=(const Vector2D& other) const
  {
    return (this->x >= other.x && this->y >= other.y);
  };

  bool operator<(const Vector2D& other) const
  {
    return (this->x < other.x && this->y < other.y);
  };

  bool operator>(const Vector2D& other) const
  {
    return (this->x > other.x && this->y > other.y);
  };

  /**
   * @brief Calculates the Euclidean length (magnitude) of the vector
   * @return Vector length as sqrt(x^2 + y^2)
   */
  double length() const
  {
    return std::sqrt((this->x * this->x) + (this->y * this->y));
  }

  /**
   * @brief Returns a normalized (unit length) copy of this vector
   * @return Normalized vector, or (0,0) if length is zero
   */
  Vector2D normalize() const
  {
    double len = this->length();
    Vector2D normal(0, 0);
    if (len == 0) {
      return {0, 0};
    }
    return {this->x / len, this->y / len};
  }

  /**
   * @brief Calculates Euclidean distance to another vector
   * @param other Target vector
   * @return Distance between this vector and other
   */
  double distanceTo(const Vector2D& other) const
  {
    return (*this - other).length();
  }

  /**
   * @brief Calculates dot product with another vector
   * @param other Second vector
   * @return Dot product (this.x * other.x + this.y * other.y)
   */
  double dot(const Vector2D& other) const
  {
    return (this->x * other.x) + (this->y * other.y);
  }

  double x = 0;  ///< X coordinate
  double y = 0;  ///< Y coordinate
};

/**
 * @brief Parses a Vector2D from binary data
 * @return Parser that deserializes two doubles into a Vector2D
 */
inline Parser<Vector2D> parseVector2D()
{
  return apply([](double x, double y) { return Vector2D {x, y}; },
               parseByte<double>(),
               parseByte<double>());
}

/**
 * @brief Serializes a Vector2D to binary data
 * @param vec Vector to serialize
 * @return ByteArray containing x and y as doubles
 */
inline ByteArray vector2DToByte(Vector2D const& vec)
{
  return byte_array_join(type_to_byte(vec.x), type_to_byte(vec.y));
}

/**
 * @brief Outputs vector to stream in readable format
 */
inline std::ostream& operator<<(std::ostream& os, const Vector2D& vec)
{
  os << "Vector2D(" << vec.x << ", " << vec.y << ")";
  return os;
}

#endif /* !VECTOR2D_HPP_ */
