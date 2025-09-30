#include "bplus_tree/node.hpp"

#include <utility>
#include <cstring>

#include "frame_manager/cache/frame_view.hpp"
#include "headers.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "exceptions.hpp"

namespace minisql {

/* Constructor for a new Node.
 * Populates the pages header. */
Node::Node(
    FrameView&& fv, Magic magic, key_size_t key_size, slot_size_t slot_size,
    page_id_t parent
) : fv_{std::move(fv)}, key_size_{key_size}, slot_size_{slot_size} {
    fv_.write<Magic>(NodeHeader::MAGIC_OFFSET, magic);
    fv_.write<key_size_t>(NodeHeader::KEY_SIZE_OFFSET, key_size_);
    fv_.write<slot_size_t>(NodeHeader::SLOT_SIZE_OFFSET, slot_size_);
    set_size(0);
    set_parent(parent);
}

/* Constructor for reading a Node from a page.
 * Reads key_size, slot_size_ and size_ eagerly. */
Node::Node(FrameView&& fv) : fv_{std::move(fv)} {
    key_size_ = fv_.view<key_size_t>(NodeHeader::KEY_SIZE_OFFSET);
    slot_size_ = fv_.view<slot_size_t>(NodeHeader::SLOT_SIZE_OFFSET);
    size_ = fv_.view<size_t>(NodeHeader::SIZE_OFFSET);
}

/* Shift slots >= start_slot to the right by steps (steps < 0 is allowed).
 * Adds steps to size_ to reflect the space added/removed. */
void Node::shift(size_t start_slot, int steps) {
    if (!steps || start_slot > size_) return;
    std::memmove(
        fv_.data() + offset(start_slot + steps),
        fv_.data() + offset(start_slot), (size_ - start_slot)  * slot_size_
    );
    set_size(size_ + steps);
}

/* Throw a NodeIncompatibilityException if the Nodes don't have matching key
 * and slot sizes. */
void Node::assert_compatibility(Node* n1, Node* n2) {
    if (n1->key_size_ != n2->key_size_ || n1->slot_size_ != n2->slot_size_)
        throw NodeIncompatibilityException(
            n1->key_size_, n2->key_size_, n1->slot_size_, n2->slot_size_
        );
}

/* Transfer count slots from the back of src onto the front of dst.
 * Slots already in dst will be shifted to make the required space. */
void Node::splice_back_to_front(Node* dst, Node* src, size_t count) {
    assert_compatibility(dst, src);
    if (count > src->size_) count = src->size_;
    dst->shift(0, count);
    std::memcpy(
        dst->fv_.data() + dst->offset(0),
        src->fv_.data() + src->offset(src->size_ - count),
        count * src->slot_size_
    );
    src->set_size(src->size_ - count);
}

/* Transfer count slots from the front of src onto the back of dst.
 * Slots left in src will be shifted to start from index 0. */
void Node::splice_front_to_back(Node* dst, Node* src, size_t count) {
    assert_compatibility(dst, src);
    if (count > src->size_) count = src->size_;
    std::memcpy(
        dst->fv_.data() + dst->offset(dst->size_),
        src->fv_.data() + src->offset(0),
        count * src->slot_size_
    );
    dst->set_size(dst->size_ + count);
    src->shift(count, - count);
}

} // namespace minisql