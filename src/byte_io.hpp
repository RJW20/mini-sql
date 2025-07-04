#ifndef MINISQL_BYTE_IO_HPP
#define MINISQL_BYTE_IO_HPP

#include <cstddef>
#include <cstring>
#include <type_traits>
#include <vector>
#include <stdexcept>

namespace minisql {

/* Byte IO
 * Class exposing static methods for reading and writing trivially copyable
 * types to std::byte arrays. */
class ByteIO {
public:
    template <typename T>
    static T read(
        const std::vector<std::byte>& bytes, std::size_t offset,
        std::size_t size = sizeof(T)
    ) {
        static_assert(
            std::is_trivially_copyable_v<T>,
            "ByteIO::read<T>: T must be trivially copyable or you need a "
            "ByteIO::read specialisation"
        );
        if (offset + size > bytes.size())
            throw std::out_of_range("ByteIO read error");
        T t;
        std::memcpy(&t, bytes.data() + offset, size);
        return t;
    }

    template <typename T>
    static void write(
        std::vector<std::byte>& bytes, std::size_t offset, const T& t
    ) {
        static_assert(
            std::is_trivially_copyable_v<T>,
            "ByteIO::write<T>: T must be trivially copyable or you need a "
            "ByteIO::write specialisation"
        );
        const std::size_t size = sizeof(T);
        if (offset + size > bytes.size())
            throw std::out_of_range("ByteIO write error");
        std::memcpy(bytes.data() + offset, &t, size);
    }
};

} // namespace minisql

#endif // MINISQL_BYTE_IO_HPP