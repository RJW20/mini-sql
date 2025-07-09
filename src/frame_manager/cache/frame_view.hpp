#ifndef MINISQL_FRAME_VIEW_HPP
#define MINISQL_FRAME_VIEW_HPP

#include <cstddef>

#include "frame_manager/cache/cache.hpp"
#include "frame_manager/cache/frame.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "byte_io.hpp"

namespace minisql {

class Cache;

/* Frame View
 * Small RAII object that provides access to a Frame. */
class FrameView {
public:
    FrameView(Cache* cache, Frame* f) : cache_{cache}, f_{f} {}
    ~FrameView();

    FrameView(const FrameView&) = delete;
    FrameView& operator=(const FrameView&) = delete;

    FrameView(FrameView&&);
    FrameView& operator=(FrameView&&);

    page_id_t pid() const noexcept { return f_->pid; }
    std::size_t page_size() const { return f_->data.size(); }

    template <typename T>
    T read(std::size_t offset, std::size_t size = sizeof(T)) const {
        return ByteIO::read<T>(f_->data, offset, size);
    }

    template <typename T>
    void write(std::size_t offset, const T& value) {
        ByteIO::write<T>(f_->data, offset, value);
        dirty_ = true;
    }

    void mark_deleted() { 
        f_->dirty = false;
        dirty_ = false;
    }

private:
    Cache* cache_;
    Frame* f_;
    bool dirty_;
};

} // namespace minisql

#endif // MINISQL_FRAME_VIEW_HPP