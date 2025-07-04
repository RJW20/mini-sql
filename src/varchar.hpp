#ifndef MINISQL_VARCHAR_HPP
#define MINISQL_VARCHAR_HPP

#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string>
#include <memory>
#include <string_view>
#include <algorithm>

#include "byte_io.hpp"

namespace minisql {

/* Varchar
 * Class for storing char arrays of fixed size (which are padded with '/0'
 * where necessary). */
class Varchar {
public:
    Varchar(const char* data, std::size_t size) {
        if (size > MAX_SIZE) {
            throw std::length_error(
                "Varchar size cannot exceed " + std::to_string(MAX_SIZE) + "."
            );
        }
        data_ = std::make_unique<char[]>(size + 1);
        std::strncpy(data_.get(), data, size);
        data_[size] = '\0';
        size_ = size;
    }

    explicit Varchar(std::size_t size) {
        if (size > MAX_SIZE) {
            throw std::length_error(
                "Varchar size cannot exceed " + std::to_string(MAX_SIZE) + "."
            );
        }
        data_ = std::make_unique<char[]>(size + 1);
        std::memset(data_.get(), 0, size + 1);
        size_ = size;
    }

    Varchar(const char c) {
        data_ = std::make_unique<char[]>(2);
        data_[0] = c;
        data_[1] = '\0';
        size_ = 1;
    }

    Varchar(const Varchar& other) {
        data_ = std::make_unique<char[]>(other.size_ + 1);
        std::strncpy(data_.get(), other.data(), other.size_);
        size_ = other.size_;
    }

    const Varchar& operator=(const Varchar& other) {
        if (this != &other) {
            std::strncpy(data_.get(), other.data(), other.size_);
            size_ = other.size_;
        }
        return *this;
    }

    bool operator==(const Varchar& other) const {
        if (size_ != other.size_) return false;
        for (std::size_t i = 0; i < size_; i++) {
            if (data_[i] != other.data_[i]) return false;
        }
        return true;
    }

    bool operator!=(const Varchar& other) const {
        return !(*this == other);
    }

    bool operator<(const Varchar& other) const {
        std::size_t min_size = std::min(size_, other.size_);
        for (std::size_t i = 0; i < min_size; i++) {
            if (data_[i] < other.data_[i]) return true;
            if (data_[i] > other.data_[i]) return false;
        }
        return size_ < other.size_;
    }

    char* data() const { return data_.get(); }
    std::size_t size() const { return size_; }
    operator char*() const { return data(); } 

private:
    std::unique_ptr<char[]> data_;
    std::size_t size_;

    static constexpr std::size_t MAX_SIZE = 64;
};

const Varchar VCHR_MIN(std::size_t(0));

template <>
Varchar ByteIO::read<Varchar>(
    const std::vector<std::byte>& bytes, std::size_t offset, std::size_t size
) {
    if (offset + size > bytes.size())
        throw std::out_of_range("ByteIO read error");
    Varchar v(size);
    std::memcpy(v.data(), bytes.data() + offset, size);
    return v;
}

template <>
void ByteIO::write<Varchar>(
    std::vector<std::byte>& bytes, std::size_t offset, const Varchar& v
) {
    const std::size_t size = v.size();
    if (offset + size > bytes.size())
        throw std::out_of_range("ByteIO write error");
    std::memcpy(bytes.data() + offset, v.data(), size);
}

} // namespace minisql

#endif // MINISQL_VARCHAR_HPP