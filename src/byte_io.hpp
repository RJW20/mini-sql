#ifndef MINISQL_BYTE_IO_HPP
#define MINISQL_BYTE_IO_HPP

#include <cstddef>
#include <cstring>
#include <type_traits>

#include "exceptions/engine_exceptions.hpp"
#include "span.hpp"
 
namespace minisql {

/* Namespace exposing functions for copying and writing trivially copyable
 * types from and to std::byte containers. */
namespace byte_io {

/* Copy T from bytes starting from the given offset.
 * Throws a ByteIOException if attempting to copy beyond the end of bytes. */
template <typename T>
T copy(
    span<std::byte> bytes, std::size_t offset, std::size_t size = sizeof(T)
) {
    static_assert(
        std::is_trivially_copyable_v<T>,
        "byte_io::copy<T>: T must be trivially copyable or you need a "
        "byte_io::copy specialisation"
    );
    if (offset + size > bytes.size())
        throw ByteIOException("copy", offset + size, bytes.size());
    T t;
    std::memcpy(&t, bytes.data() + offset, size);
    return t;
}

// Calls copy<T>.
template <typename T>
const T view(
    span<std::byte> bytes, std::size_t offset, std::size_t size = sizeof(T)
) {
    static_assert(
        std::is_trivially_copyable_v<T>,
        "byte_io::view<T>: T must be trivially copyable or you need a "
        "byte_io::view specialisation"
    );
    return copy<T>(bytes, offset);
}

/* Write T into bytes starting at the given offset.
 * Throws a ByteIOException if attempting to write beyond the end of bytes. */
template <typename T>
void write(span<std::byte> bytes, std::size_t offset, const T& t) {
    static_assert(
        std::is_trivially_copyable_v<T>,
        "byte_io::write<T>: T must be trivially copyable or you need a "
        "byte_io::write specialisation"
    );
    const std::size_t size = sizeof(T);
    if (offset + size > bytes.size())
        throw ByteIOException("write", offset + size, bytes.size());
    std::memcpy(bytes.data() + offset, &t, size);
}

} // namespace byte_io

} // namespace minisql

#endif // MINISQL_BYTE_IO_HPP