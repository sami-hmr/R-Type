#include <array>

#include "network/PacketCompresser.hpp"

#include "plugin/Byte.hpp"
#include "zlib.h"

ByteArray PacketCompresser::compress_packet(const ByteArray& data)
{
  std::array<Byte, buffer_size> buffer {};
  uLongf dest_size = static_cast<uLongf>(buffer.max_size());

  if (compress(buffer.data(), &dest_size, data.data(), data.size()) != Z_OK) {
    throw CompresserError("Failed to compress packet");
  }
  return {buffer.begin(), buffer.begin() + dest_size};
}

ByteArray PacketCompresser::uncompress_packet(const ByteArray& data)
{
  std::array<Byte, buffer_size> buffer {};
  uLongf dest_size = static_cast<uLongf>(buffer.max_size());

  if (uncompress(buffer.data(), &dest_size, data.data(), data.size()) != Z_OK) {
    throw CompresserError("Failed to uncompress packet");
  }
  return {buffer.begin(), buffer.begin() + dest_size};
}

void PacketCompresser::encrypt(ByteArray& data)
{
  data ^= encription_key;
}

void PacketCompresser::decrypt(ByteArray& data)
{
  data ^= encription_key;
}
