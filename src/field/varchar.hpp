#ifndef MINISQL_VARCHAR_HPP
#define MINISQL_VARCHAR_HPP

#include <cstddef>
#include <cstring>
#include <memory>
#include <utility>
#include <algorithm>

#include "byte_io.hpp"
#include "span.hpp"
#include "exceptions/engine_exceptions.hpp"

namespace minisql {

/* Varchar
 * Class for storing or viewing char arrays of fixed size (which are padded
 * with '/0' where necessary). */
class Varchar {
public:
    // Constructor: copies from src
    Varchar(const char* src, std::size_t size)
        : owned_{std::make_unique<char[]>(size + 1)}, data_{owned_.get()},
          size_{size} {
        if (size_) std::strncpy(data_, src, size_);
        data_[size_] = '\0';
    }

    // Constructor: view-only
    Varchar(char* external, std::size_t size)
        : owned_{nullptr}, data_{external}, size_{size} {}

    ~Varchar() = default;

    // Copy constructor: copy ownership (if applicable)
    Varchar(const Varchar& other) : size_{other.size_} {
        if (other.owned_) {
            owned_ = std::make_unique<char[]>(size_ + 1);
            data_ = owned_.get();
            std::strncpy(data_, other.data_, size_);
            data_[size_] = '\0';
        }
        else {
            owned_ = nullptr;
            data_ = other.data_;
        }
    }

    // Copy assignment: copy ownership (if applicable)
    Varchar& operator=(const Varchar& other) {
        if (this == &other) return *this;
        size_ = other.size_;
        if (other.owned_) {
            owned_ = std::make_unique<char[]>(size_ + 1);
            data_ = owned_.get();
            std::strncpy(data_, other.data_, size_);
            data_[size_] = '\0';
        }
        else {
            owned_.reset();
            data_ = other.data_;
        }
        return *this;
    }

    // Move constructor: take ownership (if applicable)
    Varchar(Varchar&& other)
        : owned_{std::move(other.owned_)}, data_{other.data_},
          size_{other.size_} {}

    // Move assignment: take ownership (if applicable)
    Varchar& operator=(Varchar&& other) {
        if (this == &other) return *this;
        owned_ = std::move(other.owned_);
        data_ = other.data_;
        size_ = other.size_;
        return *this;
    }

    // Copy and thus become the owner of the data stored
    void own_data() {
        if (owned_) return;
        owned_ = std::make_unique<char[]>(size_ + 1);
        if (size_) std::strncpy(owned_.get(), data_, size_);
        data_ = owned_.get();
        data_[size_] = '\0';
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

    char* data() const { return data_; }
    std::size_t size() const { return size_; }
    operator char*() const { return data_; } 

private:
    std::unique_ptr<char[]> owned_;
    char* data_;
    std::size_t size_;
};

// Minimum value for an object of type Varchar
const Varchar VCHR_MIN("", 0);

// ----------------------------------------------------------------------------
// Template specialisations for byte_io
// ----------------------------------------------------------------------------

template <>
inline Varchar byte_io::copy<Varchar>(
    span<std::byte> bytes, std::size_t offset, std::size_t size
) {
    if (offset + size > bytes.size())
        throw ByteIOException("copy", offset + size, bytes.size());
    return Varchar(reinterpret_cast<const char*>(bytes.data() + offset), size);
}

template <>
inline const Varchar byte_io::view<Varchar>(
    span<std::byte> bytes, std::size_t offset, std::size_t size
) {
    if (offset + size > bytes.size())
        throw ByteIOException("view", offset + size, bytes.size());
    return Varchar(reinterpret_cast<char*>(bytes.data() + offset), size);
}

template <>
inline void byte_io::write<Varchar>(
    span<std::byte> bytes, std::size_t offset, const Varchar& v
) {
    const std::size_t size = v.size();
    if (offset + size > bytes.size())
        throw ByteIOException("write", offset + size, bytes.size());
    std::memcpy(bytes.data() + offset, v.data(), size);
}

template <>
class DuplicateKeyException<Varchar> : public CursorException {
public:
    explicit DuplicateKeyException(const Varchar& key)
        : CursorException(
            "key \"" + std::string(key.data()) + "\" already exists"
        ) {}
};

} // namespace minisql

#endif // MINISQL_VARCHAR_HPP