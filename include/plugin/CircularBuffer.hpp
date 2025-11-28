#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <vector>
#include <asio.hpp>
#include <asio/impl/read.hpp>
#include <asio/ip/udp.hpp>

template<std::size_t Size = 256>
class CircularBuffer {
    public:
    using Byte = unsigned char;
    using Package = std::vector<Byte>;
    using Eof = std::vector<Byte> const &;

    std::size_t read_socket(asio::ip::udp::socket &socket) {
        std::size_t read_size = asio::read(socket,
            this->_array.data() + this->_write, asio::transfer_exactly(this->get_write_size()));

        this->_write = (this->_write + read_size) % Size;
        return read_size;
    }

    std::optional<Package> extract(Eof eof) {
        std::size_t available = this->get_available_size();
        if (available < eof.size()) {return std::nullopt;}

        Package tmp(available);
        for (size_t i = 0; i < tmp.capacity(); i++) {
            tmp[i] = this->_array[(this->_read + i) % Size];
        }
        auto needle = std::search(tmp.begin(), tmp.end(), eof.begin(), eof.end());
        if (needle == tmp.end()) {
            return std::nullopt;
        }
        this->_read = this->_read + std::distance(tmp.begin(), needle) + eof.size();
        tmp.erase(needle, tmp.end());
        return tmp;
    }

    private:

    std::size_t get_write_size() {
        return this->_read <= this->_write ? Size - this->_write : this->_read - this->_write;
    }

    std::size_t get_available_size() {
        int64_t pseudo_size = this->_write - this->_read;

        return pseudo_size > 0 ? pseudo_size : Size + pseudo_size;
    }

    std::array<Byte, Size> _array;
    std::size_t _read = 0;
    std::size_t _write = 0;

};
