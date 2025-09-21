#ifndef MINISQL_FRAME_MANAGER_HPP
#define MINISQL_FRAME_MANAGER_HPP

#include <cstddef>
#include <fstream>

#include "frame_manager/disk_manager/page_id_t.hpp"
#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/disk_manager.hpp"
#include "frame_manager/cache/cache.hpp"
#include "frame_manager/free_list/free_list.hpp"

namespace minisql {

/* Frame Manager
 * Acts as an in-memory buffer pool for database pages. Handles LRU eviction,
 * dirty‑page flushing, and page allocation/reuse. */
class FrameManager {
public:
    FrameManager(
        std::fstream& file, std::streamoff base_offset,
        std::size_t page_size, page_id_t page_count,
        std::size_t cache_capacity, page_id_t first_free_list_block = nullpid
    ) : disk_{file, base_offset, page_size, page_count},
        cache_{disk_, cache_capacity},
        free_list_{cache_, first_free_list_block} {}
    ~FrameManager() = default;

    FrameManager(const FrameManager&) = delete;
    FrameManager& operator=(const FrameManager&) = delete;

    FrameView pin(page_id_t pid) { return cache_.pin(pid); }

    FrameView allocate() {
        if (!free_list_.empty()) return cache_.pin(free_list_.pop_back());
        page_id_t pid = disk_.page_count();
        disk_.extend();
        return cache_.pin(pid);
    }
    void deallocate(page_id_t pid) { free_list_.push_back(pid); }

    void flush_all() { cache_.flush_all(); }

    page_id_t page_count() const noexcept { return disk_.page_count(); }
    page_id_t first_free_list_block() const noexcept {
        return free_list_.first_free_list_block();
    }

private:
    DiskManager disk_;
    Cache cache_;
    FreeList free_list_;
};

} // namespace minisql

#endif // MINISQL_FRAME_MANAGER_HPP