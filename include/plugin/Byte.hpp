#pragma once

#include <algorithm>
#include <bit>
#include <concepts>
#include <string>
#include <vector>
#include "CustomException.hpp"

using Byte = unsigned char;
using ByteArray = std::vector<Byte>;

template <typename T>
concept serializable = std::constructible_from<T, ByteArray>;

template <typename T>
concept unserializable = requires (T t) {{ t.to_bytes() } -> std::same_as<ByteArray>;};

template <typename T>
concept bytable = serializable<T> && unserializable<T>;

#define EMPTY_BYTE_CONSTRUCTOR(classname) \
    classname(ByteArray const& array) \
    { \
        Result<classname> r = \
            apply(([]() { return (classname){}; }))(Rest(std::string(array.begin(), array.end()))); \
    if (r.index() == ERROR) { \
        throw InvalidPackage(#classname); \
    } \
    *this = std::get<SUCCESS>(r).value; \
    } \

#define DEFAULT_BYTE_CONSTRUCTOR(classname, construct, ...) \
    classname(ByteArray const& array) \
    { \
        Result<classname> r = \
            apply((construct), __VA_ARGS__)(Rest(std::string(array.begin(), array.end()))); \
    if (r.index() == ERROR) { \
        throw InvalidPackage(#classname); \
    } \
    *this = std::get<SUCCESS>(r).value; \
    } \

ByteArray operator+(ByteArray first, ByteArray const &second);

template<std::same_as<ByteArray>... Args>
ByteArray byte_array_join(Args... arrays) {
    return (arrays + ...);
}

#define DEFAULT_SERIALIZE(...) \
    ByteArray to_bytes() { \
        return (byte_array_join(__VA_ARGS__)); \
    } \

template <typename T>
ByteArray type_to_byte(T v, std::endian endian = std::endian::big) {
    ByteArray tmp(&v, &v + sizeof(v));
    if (endian != std::endian::native) {
        std::reverse(tmp.begin(), tmp.end());
    }
    return tmp;
}

ByteArray string_to_byte(std::string const &str);

CUSTOM_EXCEPTION(InvalidPackage)
