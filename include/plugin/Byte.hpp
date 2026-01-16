#pragma once

#include <algorithm>
#include <bit>
#include <concepts>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "CustomException.hpp"
#include "Json/JsonParser.hpp"

/**
 * @file Byte.hpp
 * @brief Binary serialization system for network communication and persistence
 *
 * This file provides the foundation for the engine's binary protocol, enabling:
 * - Component/event serialization for networking
 * - Type-safe byte array construction and parsing
 * - Endianness-aware data conversion
 * - Support for primitives, containers, and custom types
 */

using Byte = unsigned char;
using ByteArray = std::vector<Byte>;

/**
 * @concept serializable
 * @brief Type can be constructed from a ByteArray
 *
 * Types satisfying this concept must have a constructor accepting ByteArray,
 * typically used for deserialization from network or file data.
 *
 * @see DEFAULT_BYTE_CONSTRUCTOR macro
 */
template<typename T>
concept serializable = std::constructible_from<T, ByteArray>;

/**
 * @concept unserializable
 * @brief Type can be converted to a ByteArray
 *
 * Types satisfying this concept must provide a to_bytes() method returning
 * ByteArray, used for serialization to network or file data.
 *
 * @see DEFAULT_SERIALIZE macro
 */
template<typename T>
concept unserializable = requires(T t) {
  {
    t.to_bytes()
  } -> std::same_as<ByteArray>;
};

/**
 * @concept bytable
 * @brief Type supports bidirectional byte conversion
 *
 * Types satisfying this concept can be both serialized and deserialized,
 * making them suitable for network transmission and persistence. All
 * components and events must satisfy this concept.
 *
 * @see component concept
 * @see event concept
 */
template<typename T>
concept bytable = serializable<T> && unserializable<T>;

/**
 * @def EMPTY_BYTE_CONSTRUCTOR
 * @brief Generates a default ByteArray constructor for empty structures
 * @param classname Name of the class/struct
 *
 * Use for simple types with no fields that still need byte serialization.
 * Attempts to parse as empty structure, throws InvalidPackage on failure.
 */
#define EMPTY_BYTE_CONSTRUCTOR(classname) \
  classname(ByteArray const& array) \
  { \
    Result<classname> r = apply(([]() { return classname{}; }))( \
        Rest(std::string(array.begin(), array.end()))); \
    if (r.index() == ERR) { \
      throw InvalidPackage(#classname); \
    } \
    *this = std::get<SUCCESS>(r).value; \
  }

/**
 * @def DEFAULT_BYTE_CONSTRUCTOR
 * @brief Generates a ByteArray constructor using parser combinators
 * @param classname Name of the class/struct
 * @param construct Lambda that constructs the object from parsed fields
 * @param ... Parser combinators for each field
 *
 * Uses the ByteParser library to deserialize fields in order. Parsers are
 * applied sequentially, and results are passed to the construct lambda.
 * Throws InvalidPackage with detailed error info on parse failure.
 *
 * @code
 * struct Position {
 *   double x, y;
 *   DEFAULT_BYTE_CONSTRUCTOR(
 *     Position,
 *     ([](double x, double y) { return Position{x, y}; }),
 *     parseByte<double>(),
 *     parseByte<double>()
 *   )
 * };
 * @endcode
 */
#define DEFAULT_BYTE_CONSTRUCTOR(classname, construct, ...) \
  classname(ByteArray const& array) \
  { \
    Result<classname> r = apply((construct), __VA_ARGS__)(Rest(array)); \
    if (r.index() == ERR) { \
      auto const& err = std::get<ERR>(r); \
      throw InvalidPackage(std::format("{}: {}, {}, line {} col {}", \
                                       #classname, \
                                       err.context, \
                                       err.message, \
                                       err.rest.lines, \
                                       err.rest.columns)); \
    } \
    *this = std::get<SUCCESS>(r).value; \
  }

/**
 * @brief ByteArray concatenation operator
 * @param first First array (copied)
 * @param second Second array to append
 * @return New ByteArray containing first + second
 */
ByteArray operator+(ByteArray first, ByteArray const& second);

/**
 * @brief ByteArray append operator
 * @param first Array to modify
 * @param second Array to append
 * @return Reference to modified first array
 */
ByteArray& operator+=(ByteArray& first, ByteArray const& second);

std::vector<ByteArray> operator/(ByteArray const&, std::size_t);
ByteArray operator^(ByteArray const&, std::size_t);
ByteArray& operator^=(ByteArray&, std::size_t);

/**
 * @brief Joins multiple ByteArrays into one
 * @param arrays Variable number of ByteArrays to concatenate
 * @return Single ByteArray containing all input arrays in order
 *
 * Uses fold expression for efficient concatenation.
 */
template<std::same_as<ByteArray>... Args>
ByteArray byte_array_join(Args... arrays)
{
  return (arrays + ...);
}

/**
 * @def DEFAULT_SERIALIZE
 * @brief Generates a to_bytes() method by joining field serializations
 * @param ... ByteArray expressions for each field
 *
 * Concatenates the ByteArrays produced by each argument using byte_array_join.
 * Fields are serialized in the order specified.
 *
 * @code
 * struct Position {
 *   double x, y;
 *   DEFAULT_SERIALIZE(type_to_byte(x), type_to_byte(y))
 * };
 * @endcode
 */
#define DEFAULT_SERIALIZE(...) \
  ByteArray to_bytes() const \
  { \
    return (byte_array_join(__VA_ARGS__)); \
  }

/**
 * @brief Converts a primitive type to ByteArray with endianness control
 * @tparam T Type to convert (must be trivially copyable)
 * @param v Value to convert
 * @param endian Target endianness (default: big-endian)
 * @return ByteArray containing the binary representation
 *
 * Uses std::bit_cast for safe type punning. Reverses byte order if target
 * endianness differs from native. Network protocol uses big-endian.
 */
template<typename T>
ByteArray type_to_byte(T v, std::endian endian = std::endian::big)
{
  static_assert(sizeof(T) <= 64, "Type too large for type_to_byte");
  ByteArray tmp(sizeof(T));
  unsigned char* ptr = reinterpret_cast<unsigned char*>(&v);
  std::copy(ptr, ptr + sizeof(T), tmp.begin());

  if (endian != std::endian::native) {
    std::reverse(tmp.begin(), tmp.end());
  }
  return tmp;
}

/**
 * @brief Creates a type-to-byte conversion function for a specific type
 * @tparam Type Type to create converter for
 * @return Function that converts Type values to ByteArray
 *
 * Helper for creating conversion functions for use with higher-order functions.
 */
template<typename Type>
constexpr std::function<ByteArray(Type const&)> TTB_FUNCTION()
{
  return [](Type const& v) { return type_to_byte(v); };
}

/**
 * @brief Creates a custom serialization function wrapper
 * @tparam Type Type to serialize
 * @tparam Args Additional argument types for the serializer
 * @param f Serialization function
 * @param args Additional arguments to bind
 * @return Function that serializes Type values to ByteArray
 */
template<typename Type, typename... Args>
constexpr std::function<ByteArray(Type const&)> SERIALIZE_FUNCTION(auto&& f,
                                                                   Args... args)
{
  return [f, args...](Type const& v) { return f(v, args...); };
}

/**
 * @brief Serializes a vector to ByteArray
 * @tparam T Element type
 * @param v Vector to serialize
 * @param f Function to serialize each element
 * @return ByteArray: size (uint32) followed by serialized elements
 */
template<typename T>
ByteArray vector_to_byte(std::vector<T> const& v,
                         std::function<ByteArray(T const&)> f)
{
  ByteArray r = type_to_byte<std::uint32_t>(v.size());
  for (auto const& it : v) {
    r = r + f(it);
  }
  return r;
}

/**
 * @brief Serializes a key-value pair
 * @tparam Key Key type
 * @tparam Value Value type
 * @param p Pair to serialize
 * @param f1 Function to serialize keys
 * @param f2 Function to serialize values
 * @return ByteArray containing serialized key followed by value
 */
template<typename Key, typename Value>
ByteArray pair_to_byte(std::pair<Key, Value> p,
                       std::function<ByteArray(Key const&)> f1,
                       std::function<ByteArray(Value const&)> f2)
{
  return f1(p.first) + f2(p.second);
}

/**
 * @brief Serializes an unordered_map to ByteArray
 * @tparam Key Key type
 * @tparam Value Value type
 * @param m Map to serialize
 * @param f1 Function to serialize keys
 * @param f2 Function to serialize values
 * @return ByteArray: size followed by serialized key-value pairs
 */
template<typename Key, typename Value>
ByteArray map_to_byte(std::unordered_map<Key, Value> const& m,
                      std::function<ByteArray(Key const&)> f1,
                      std::function<ByteArray(Value const&)> f2)
{
  return vector_to_byte(std::vector<std::pair<Key, Value>>(m.begin(), m.end()),
                        std::function([f1, f2](std::pair<Key, Value> const& p)
                                      { return pair_to_byte(p, f1, f2); }));
}

/**
 * @brief Serializes an optional value
 * @tparam T Value type
 * @param m Optional to serialize
 * @param f1 Function to serialize the value if present
 * @return ByteArray: bool (has_value) followed by value if present
 */
template<typename T>
ByteArray optional_to_byte(std::optional<T> const& m,
                           std::function<ByteArray(T)> f1)
{
  if (!m.has_value()) {
    return type_to_byte<bool>(false);
  }
  return type_to_byte<bool>(true) + f1(m.value());
}

/**
 * @brief Serializes a string to ByteArray
 * @param str String to serialize
 * @return ByteArray: length (uint32) followed by UTF-8 bytes
 */
ByteArray string_to_byte(std::string const& str);

/**
 * @brief Serializes a JsonValue (variant type)
 * @param v JsonValue to serialize
 * @return ByteArray: type index followed by serialized value
 */
ByteArray json_value_to_byte(JsonValue const& v);

/**
 * @brief Serializes a JsonObject (map of string to JsonValue)
 * @param object JsonObject to serialize
 * @return ByteArray containing the entire object structure
 */
ByteArray json_object_to_byte(JsonObject const& object);

/**
 * @class InvalidPackage
 * @brief Exception thrown when byte deserialization fails
 *
 * Used by DEFAULT_BYTE_CONSTRUCTOR when parsing fails. Contains error
 * context, message, and position information for debugging.
 */
CUSTOM_EXCEPTION(InvalidPackage)
