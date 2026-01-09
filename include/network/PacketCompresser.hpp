#pragma once

#include "CustomException.hpp"
#include "plugin/Byte.hpp"

class PacketCompresser
{
  static constexpr Byte encription_key = 67;
  static constexpr std::size_t buffer_size = 20000;

public:
  static ByteArray compress_packet(ByteArray const&);
  static ByteArray uncompress_packet(ByteArray const&);

  static void encrypt(ByteArray&);
  static void decrypt(ByteArray&);
};

CUSTOM_EXCEPTION(CompresserError)
