#include "plugin/Byte.hpp"
#include <sys/types.h>

ByteArray operator+(ByteArray first, ByteArray const &second) {
    first.insert(first.end(), second.begin(), second.end());
    return first;
}

ByteArray &operator+=(ByteArray &first, ByteArray const &second) {
    first.insert(first.end(), second.begin(), second.end());
    return first;
}

ByteArray string_to_byte(std::string const &str) {
    return type_to_byte<u_int32_t>(str.size()) + ByteArray(str.begin(), str.end());
}
