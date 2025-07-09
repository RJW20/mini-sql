#include "frame_manager/cache/frame_view.hpp"

#include "frame_manager/cache/cache.hpp"

namespace minisql {

// Unpin the Frame.
FrameView::~FrameView() {
    if (cache_) cache_->unpin(f_->pid, dirty_);
}

// Move constructor needs to move all resources from other.
FrameView::FrameView(FrameView&& other)
    : cache_{other.cache_}, f_{other.f_}, dirty_{other.dirty_} {
    other.cache_ = nullptr;
    other.f_ = nullptr;
    other.dirty_ = false;
}

// Move assignment needs to unpin the current Frame and move all resources.
FrameView& FrameView::operator=(FrameView&& other) {
    if (this != &other) {
        cache_->unpin(f_->pid, dirty_);
        cache_ = other.cache_;
        other.cache_ = nullptr;
        f_ = other.f_;
        other.f_ = nullptr;
        dirty_ = other.dirty_;
        other.dirty_ = false;
    }
    return *this;
}

} // namespace minisql