#include "frame_manager/free_list/free_list.hpp"

#include <utility>

#include "frame_manager/disk_manager/page_id_t.hpp"
#include "frame_manager/free_list/free_list_block.hpp"

namespace minisql {

// Return the pid of a free page.
page_id_t FreeList::pop_back() {

    // Get the first block of the free list
    FreeListBlock current_block{cache_.pin(first_free_list_block_)};
    page_id_t next_block_pid = current_block.next_block();

    // If the free list is only one block then delete it if needed
    if (next_block_pid == nullpid) {
        if (!current_block.empty()) return current_block.pop_back();
        current_block.mark_deleted();
        page_id_t free_pid = first_free_list_block_;
        first_free_list_block_ = nullpid;
        return free_pid;
    }

    // Find the end block of the free list
    FreeListBlock previous_block = std::move(current_block);
    FrameView fv = cache_.pin(next_block_pid);
    current_block = FreeListBlock{std::move(fv)};
    next_block_pid = current_block.next_block();
    while (next_block_pid != nullpid) {
        previous_block = std::move(current_block);
        current_block = FreeListBlock{cache_.pin(next_block_pid)};
        next_block_pid = current_block.next_block();
    }

    // Return the page_id_t at the end of the block or delete it
    if (!current_block.empty()) return current_block.pop_back();
    current_block.mark_deleted();
    page_id_t free_pid = previous_block.next_block();
    previous_block.set_next_block(nullpid);
    return free_pid;
}

// Add a pid to the Free List.
void FreeList::push_back(page_id_t pid) {

    // If the free list is empty then create the first block
    if (first_free_list_block_ == nullpid) {
        first_free_list_block_ = pid;
        FreeListBlock{cache_.pin(first_free_list_block_), true};
        return;
    }

    // Find the end block of the free list
    FreeListBlock current_block{cache_.pin(first_free_list_block_)};
    page_id_t next_block_pid = current_block.next_block();
    while (next_block_pid != nullpid) {
        current_block = FreeListBlock{cache_.pin(next_block_pid)};
        next_block_pid = current_block.next_block();
    }

    // Add the given pid to the end block or create a new one
    if (!current_block.full()) {
        current_block.push_back(pid);
        return;
    }
    current_block.set_next_block(pid);
    FreeListBlock{cache_.pin(pid), true};
}

} // namespace minisql