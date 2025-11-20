#ifndef MINISQL_CACHE_HPP
#define MINISQL_CACHE_HPP

#include <cstddef>
#include <list>
#include <unordered_map>
#include <vector>

#include "frame_manager/cache/frame.hpp"
#include "frame_manager/disk_manager/disk_manager.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"

namespace minisql {

class FrameView;

/* Cache
 * Holds Frames in memory. Maps page_id_t's to those Frames and manages LRU
 * eviction and dirty page flushing. */
class Cache {
public:
    Cache(DiskManager& disk, std::size_t capacity)
        : disk_{disk}, capacity_{capacity}, frames_{capacity},
        next_free_fid_{0} {
        for (Frame& f : frames_) f.lru_it = lru_.end();
    }
    ~Cache() { flush_all(); }

    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;

    FrameView pin(page_id_t pid);
    void unpin(page_id_t pid, bool dirty);

    void flush_all() { for (Frame& f : frames_) flush(f); };

    std::size_t capacity() const noexcept { return capacity_; }

private:
    std::size_t get_free_fid();

    void flush(Frame& f);

    DiskManager& disk_;
    const std::size_t capacity_;
    std::vector<Frame> frames_;
    std::unordered_map<page_id_t, std::size_t> map_;
    std::list<std::size_t> lru_;
    std::size_t next_free_fid_;
};

} // namespace minisql

#endif // MINISQL_CACHE_HPP