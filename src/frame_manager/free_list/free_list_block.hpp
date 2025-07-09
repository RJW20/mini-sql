#ifndef MINISQL_FREE_LIST_BLOCK_HPP
#define MINISQL_FREE_LIST_BLOCK_HPP

#include <cstdint>
#include <cstddef>
#include <utility>

#include "frame_manager/cache/frame_view.hpp"
#include "headers.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "exceptions.hpp"

namespace minisql {

/* Free List Block
 * Acts as an interface over a page, consisting of a FreeListBlockHeader
 * followed by a stack of pids. */
class FreeListBlock {
public:
    FreeListBlock(FrameView fv, bool new_ = false) : fv_{std::move(fv)} {
        if (new_) {
            fv_.write<Magic>(
                FreeListBlockHeader::MAGIC_OFFSET, Magic::FREE_LIST_BLOCK
            );
            set_stack_pointer(FreeListBlockHeader::SIZE);
            set_next_block(nullpid);
        }
        else {
            Magic magic = fv_.read<Magic>(FreeListBlockHeader::MAGIC_OFFSET);
            if (magic != Magic::FREE_LIST_BLOCK)
                throw InvalidMagicException(magic);
        }
    }

    page_id_t next_block() const { 
        return fv_.read<page_id_t>(FreeListBlockHeader::NEXT_BLOCK_OFFSET);
    }
    void set_next_block(page_id_t pid) {
        fv_.write<page_id_t>(FreeListBlockHeader::NEXT_BLOCK_OFFSET, pid);
    }

    page_id_t pop_back() {
        std::size_t sp = stack_pointer() - sizeof(page_id_t);
        set_stack_pointer(sp);
        return fv_.read<page_id_t>(sp);
    }
    void push_back(page_id_t pid) {
        std::size_t sp = stack_pointer();
        fv_.write<page_id_t>(sp, pid);
        set_stack_pointer(sp + sizeof(page_id_t));
    }

    bool empty() const { return stack_pointer() == FreeListBlockHeader::SIZE; }
    bool full() const {
        return stack_pointer() + sizeof(page_id_t) > fv_.page_size();
    }
    void mark_deleted() { fv_.mark_deleted(); }

private:
    FrameView fv_;

    std::uint16_t stack_pointer() const { 
        return fv_.read<std::uint16_t>(
            FreeListBlockHeader::STACK_POINTER_OFFSET
        );
    }
    void set_stack_pointer(std::uint16_t stack_pointer) {
        fv_.write<std::uint16_t>(
            FreeListBlockHeader::STACK_POINTER_OFFSET, stack_pointer
        );
    }
};

} // namespace minisql

#endif // MINISQL_FREE_LIST_BLOCK_HPP