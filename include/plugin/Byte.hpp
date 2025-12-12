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

using Byte = unsigned char;
using ByteArray = std::vector<Byte>;

template<typename T>
concept serializable = std::constructible_from<T, ByteArray>;

template<typename T>
concept unserializable = requires(T t) {
  { t.to_bytes() } -> std::same_as<ByteArray>;
};

template<typename T>
concept bytable = serializable<T> && unserializable<T>;

#define EMPTY_BYTE_CONSTRUCTOR(classname) \
  classname(ByteArray const& array) \
  { \
    Result<classname> r = apply(([]() { return (classname) {}; }))( \
        Rest(std::string(array.begin(), array.end()))); \
    if (r.index() == ERROR) { \
      throw InvalidPackage(#classname); \
    } \
    *this = std::get<SUCCESS>(r).value; \
  }

#define DEFAULT_BYTE_CONSTRUCTOR(classname, construct, ...) \
  classname(ByteArray const& array) \
  { \
    Result<classname> r = apply((construct), __VA_ARGS__)( \
        Rest(std::string(array.begin(), array.end()))); \
    if (r.index() == ERROR) { \
      throw InvalidPackage(#classname); \
    } \
    *this = std::get<SUCCESS>(r).value; \
  }

ByteArray operator+(ByteArray first, ByteArray const& second);
ByteArray& operator+=(ByteArray& first, ByteArray const& second);

template<std::same_as<ByteArray>... Args>
ByteArray byte_array_join(Args... arrays)
{
  return (arrays + ...);
}

#define DEFAULT_SERIALIZE(...) \
  ByteArray to_bytes() const \
  { \
    return (byte_array_join(__VA_ARGS__)); \
  }

template<typename T>
ByteArray type_to_byte(T v, std::endian endian = std::endian::big)
{
  ByteArray tmp(sizeof(T));
  auto raw = std::bit_cast<std::array<unsigned char, sizeof(T)>>(v);
  std::copy(raw.begin(), raw.end(), tmp.begin());

  if (endian != std::endian::native) {
    std::reverse(tmp.begin(), tmp.end());
  }
  return tmp;
}

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

template<typename Key, typename Value>
ByteArray pair_to_byte(std::pair<Key, Value> p,
                       std::function<ByteArray(Key const&)> f1,
                       std::function<ByteArray(Value const&)> f2)
{
  return f1(p.first) + f2(p.second);
}

template<typename Key, typename Value>
ByteArray map_to_byte(std::unordered_map<Key, Value> const& m,
                      std::function<ByteArray(Key const&)> f1,
                      std::function<ByteArray(Value const&)> f2)
{
  return vector_to_byte(std::vector<std::pair<Key, Value>>(m.begin(), m.end()),
                        std::function([f1, f2](std::pair<Key, Value> const& p)
                                      { return pair_to_byte(p, f1, f2); }));
}

template<typename T>
ByteArray optional_to_byte(std::optional<T> const& m,
                           std::function<ByteArray(T)> f1)
{
  if (!m.has_value()) {
    return type_to_byte<bool>(false);
  }
  return type_to_byte<bool>(true) + f1(m.value());
}

ByteArray string_to_byte(std::string const& str);

CUSTOM_EXCEPTION(InvalidPackage)
