#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <vector>

#include <asio.hpp>
#include <asio/basic_stream_socket.hpp>
#include <asio/impl/read.hpp>
#include <asio/ip/udp.hpp>

#include "plugin/Byte.hpp"

#define SIZE 256

template<std::size_t Size = SIZE>
class CircularBuffer
{
public:
  using Package = std::vector<Byte>;
  using Eof = std::vector<Byte> const&;

  std::size_t read_socket(asio::ip::udp::socket& socket,
                          asio::ip::udp::endpoint& sender,
                          std::error_code& ec)
  {
    // asio::io_context io;
    // asio::basic_stream_socket<asio::ip::udp> test(io);

    std::size_t read_size =
        socket.receive_from(asio::buffer(this->_array.data() + this->_write,
                                         this->get_write_size()),
                            sender,
                            0,
                            ec);

    this->_write = (this->_write + read_size) % Size;
    return read_size;
  }

  std::optional<Package> extract(Eof eof)
  {
    std::size_t available = this->get_available_size();
    if (available < eof.size()) {
      return std::nullopt;
    }

    Package tmp(available);
    for (size_t i = 0; i < tmp.size(); i++) {
      tmp[i] = this->_array[(this->_read + i) % Size];
    }
    auto needle = std::search(tmp.begin(), tmp.end(), eof.begin(), eof.end());
    if (needle == tmp.end()) {
      return std::nullopt;
    }
    this->_read = (this->_read + std::distance(tmp.begin(), needle) + eof.size()) % Size;
    tmp.erase(needle, tmp.end());
    return tmp;
  }

private:
  std::size_t get_write_size() const
  {
      if (_write >= _read) {
          return Size - 1 - _write;
      }
      // When write has wrapped: space is from _write to _read-1
      return _read - _write - 1;
  }

  std::size_t get_available_size()
  {
      if (_write >= _read) {
          return _write - _read;
      }
      return Size - (_read - _write);
  }

  std::array<Byte, Size> _array;
  std::size_t _read = 0;
  std::size_t _write = 0;
};
