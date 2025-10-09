#include "frame_manager/cache/cache.hpp"

#include <cstddef>

#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "frame_manager/cache/frame.hpp"
#include "exceptions/engine_exceptions.hpp"

namespace minisql {

/* Pin the page at pid into a Frame and return a FrameView containing a pointer
 * to it.
 * If the page is already pinned into a Frame then the pin_count in that Frame
 * is incremented instead. */
FrameView Cache::pin(page_id_t pid) {

    auto it = map_.find(pid);
    if (it != map_.end()) {
        Frame& f = frames_[it->second];
        if (!f.pin_count) {
            lru_.erase(f.lru_it);
            f.lru_it = lru_.end();
        }
        f.pin_count++;
        return FrameView{this, &f};
    }

    std::size_t fid = get_free_fid();
    Frame& f = frames_[fid];
    f.pid = pid;
    f.data.resize(disk_.page_size());
    disk_.read(f.pid, f.data.data());
    f.pin_count = 1;
    map_[pid] = fid;
    return FrameView{this, &f};
}

/* Unpin the page at pid from its Frame.
 * If the Frame has pin_count > 1 then the pin_count is decremented only. */
void Cache::unpin(page_id_t pid, bool dirty) {

    auto it = map_.find(pid);
    if (it == map_.end()) throw CacheUnpinException(pid, "not in cache");

    Frame& f = frames_[it->second];
    if (!f.pin_count) throw CacheUnpinException(pid, "pin_count already 0");

    f.dirty |= dirty;
    if (!(--f.pin_count)) {
        lru_.push_front(it->second);
        f.lru_it = lru_.begin();
    }
}

/* Return the index of a free Frame.
 * If needed, evicts a Frame from lru_ and flushes the page contained within it
 * to the disk. */
std::size_t Cache::get_free_fid() {

    if (next_free_fid_ < capacity_) return next_free_fid_++;

    if (lru_.empty()) throw CacheCapacityException();

    std::size_t free_fid = lru_.back();
    lru_.pop_back();
    Frame& f = frames_[free_fid];
    f.lru_it = lru_.end();
    flush(f);
    map_.erase(f.pid);
    return free_fid;
}

// Flush the given Frame to the disk if it is dirty.
void Cache::flush(Frame& f) {
    if (!f.dirty) return;
    disk_.write(f.pid, f.data.data());
    f.dirty = false;
}

} // namespace minisql