#include "bplus_tree/node.hpp"

#include <utility>
#include <cstring>

#include "frame_manager/cache/frame_view.hpp"
#include "headers.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "varchar.hpp"

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

/* Transfer the back count slots onto the front of node.
 * Slots already in node will be shifted to make the required space. */
void Node::transfer_to_front(Node* node, size_t count) {
    if (count > size_) count = size_;
    node->shift(0, count);
    std::memcpy(
        node->fv_.data() + node->offset(0), fv_.data() + offset(size_ - count),
        count * slot_size_
    );
    set_size(size_ - count);
}

/* Transfer the front count slots onto the back of node.
 * Slots left in this node will be shifted so they start from index 0. */
void Node::transfer_to_back(Node* node, size_t count) {
    if (count > size_) count = size_;
    std::memcpy(
        node->fv_.data() + node->offset(node->size_), fv_.data() + offset(0),
        count * slot_size_
    );
    node->set_size(node->size_ + count);
    shift(count, - count);
}

} // namespace minisql