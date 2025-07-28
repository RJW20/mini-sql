#ifndef MINISQL_HEADERS_HPP
#define MINISQL_HEADERS_HPP

#include <cstdint>
#include <cstddef>

#include "frame_manager/disk_manager/page_id_t.hpp"

namespace minisql {

/* Magic
 * Indicates the type and structure of a page. */
enum class Magic : std::uint8_t {
    FREE_LIST_BLOCK = 0,
    INTERNAL_NODE = 1,
    LEAF_NODE = 2,
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
    using stack_pointer_t = std::uint16_t;

    static constexpr std::size_t STACK_POINTER_OFFSET = BaseHeader::SIZE;
    static constexpr std::size_t NEXT_BLOCK_OFFSET =
        STACK_POINTER_OFFSET + sizeof(stack_pointer_t);
    static constexpr std::size_t SIZE = NEXT_BLOCK_OFFSET + sizeof(page_id_t);
};

/* NodeHeader Structure:
 * - BaseHeader
 * - std::uint8_t key_size
 * - std::uint16_t slot_size
 * - std::uint16_t size
 * - page_id_t parent */
struct NodeHeader : public BaseHeader {
    using key_size_t = std::uint8_t;
    using slot_size_t = std::uint16_t;
    using size_t = std::uint16_t;

    static constexpr std::size_t KEY_SIZE_OFFSET = BaseHeader::SIZE;
    static constexpr std::size_t SLOT_SIZE_OFFSET =
        KEY_SIZE_OFFSET + sizeof(key_size_t);
    static constexpr std::size_t SIZE_OFFSET =
        SLOT_SIZE_OFFSET + sizeof(slot_size_t);
    static constexpr std::size_t PARENT_OFFSET = SIZE_OFFSET + sizeof(size_t);
    static constexpr std::size_t SIZE = PARENT_OFFSET + sizeof(page_id_t);
};

/* InternalNodeHeader Structure
 * - NodeHeader
 * - page_id_t first_child */
struct InternalNodeHeader : public NodeHeader {
    static constexpr std::size_t FIRST_CHILD_OFFSET = NodeHeader::SIZE;
    static constexpr std::size_t SIZE = FIRST_CHILD_OFFSET + sizeof(page_id_t);
};

/* LeafNodeHeader Structure
 * - NodeHeader
 * - page_id_t next_leaf */
struct LeafNodeHeader : public NodeHeader {
    static constexpr std::size_t NEXT_LEAF_OFFSET = NodeHeader::SIZE;
    static constexpr std::size_t SIZE = NEXT_LEAF_OFFSET + sizeof(page_id_t);
};

} // namespace minisql

#endif // MINISQL_HEADERS_HPP