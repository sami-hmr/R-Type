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
#include "plugin/Byte.hpp"

class Vector2D
{
public:
  Vector2D() = default;
  Vector2D(double x, double y)
      : x(x)
      , y(y) {};
  Vector2D(JsonVariant const& variant);
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

  Vector2D &operator*=(Vector2D other)
  {
    this->x *= other.x;
    this->y *= other.y;
    return *this;
  }

  Vector2D &operator*=(double scalar)
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

  double length() const
  {
    return std::sqrt((this->x * this->x) + (this->y * this->y));
  }

  Vector2D normalize() const
  {
    double len = this->length();
    Vector2D normal(0, 0);
    if (len == 0) {
      return {0, 0};
    }
    return {this->x / len, this->y / len};
  }

  double x = 0;
  double y = 0;
};

inline Parser<Vector2D>parseVector2D()
{
  return apply(
      [](double x, double y) { return Vector2D{x, y}; },
      parseByte<double>(),
      parseByte<double>());
}

inline ByteArray vector2DToByte(Vector2D const &vec) {
    return byte_array_join(type_to_byte(vec.x), type_to_byte(vec.y));
}

inline std::ostream& operator<<(std::ostream& os, const Vector2D& vec)
{
  os << "Vector2D(" << vec.x << ", " << vec.y << ")";
  return os;
}

#endif /* !VECTOR2D_HPP_ */
