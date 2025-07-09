#ifndef MINISQL_HEADERS_HPP
#define MINISQL_HEADERS_HPP

#include <cstdint>
#include <cstddef>

#include "frame_manager/disk_manager/page_id_t.hpp"

namespace minisql {

/* Magic
 * Indicates the type and structure of a Page. */
enum class Magic : std::uint8_t {
    FREE_LIST_BLOCK = 0,
};

/* BaseHeader Structure:
 * - Magic magic */
struct BaseHeader {
    static constexpr std::size_t MAGIC_OFFSET = 0;
    static constexpr std::size_t SIZE = MAGIC_OFFSET + sizeof(Magic);
};

/* FreeListBlockHeader Structure:
 * - BaseHeader
 * - std::uint16_t stack_pointer
 * - page_id_t next_block */
struct FreeListBlockHeader : public BaseHeader {
    static constexpr std::size_t STACK_POINTER_OFFSET = BaseHeader::SIZE;
    static constexpr std::size_t NEXT_BLOCK_OFFSET =
        STACK_POINTER_OFFSET + sizeof(std::uint16_t);
    static constexpr std::size_t SIZE = NEXT_BLOCK_OFFSET + sizeof(page_id_t);
};

} // namespace minisql

#endif // MINISQL_HEADERS_HPP