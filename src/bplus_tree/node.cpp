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
template <typename Key>
Node<Key>::Node(
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
template <typename Key>
Node<Key>::Node(FrameView&& fv) : fv_{std::move(fv)} {
    key_size_ = fv_.view<key_size_t>(NodeHeader::KEY_SIZE_OFFSET);
    slot_size_ = fv_.view<slot_size_t>(NodeHeader::SLOT_SIZE_OFFSET);
    size_ = fv_.view<size_t>(NodeHeader::SIZE_OFFSET);
}

/* Shift slots >= start_slot to the right by steps (steps < 0 is allowed).
 * Adds steps to size_ to reflect the space added/removed. */
template <typename Key>
void Node<Key>::shift(size_t start_slot, int steps) {
    if (!steps || start_slot > size_) return;
    std::memmove(
        fv_.data() + offset(start_slot + steps),
        fv_.data() + offset(start_slot), (size_ - start_slot)  * slot_size_
    );
    set_size(size_ + steps);
}

/* Transfer slots >= start_slot onto the beginning of right_node.
 * Slots already in right_node will be shifted to make the required space. */
template <typename Key>
void Node<Key>::transfer_right(size_t start_slot, Node<Key>* right_node) {
    if (start_slot > size_) return;
    right_node->shift(0, size_ - start_slot);
    std::memcpy(
        right_node->fv_.data() + right_node->offset(0),
        fv_.data() + offset(start_slot), (size_ - start_slot) * slot_size_
    );
    set_size(start_slot);
}

/* Transfer slots < end_slot onto the end of left_node.
 * Slots left in this Node will be shifted so they start from index 0. */
template <typename Key>
void Node<Key>::transfer_left(size_t end_slot, Node<Key>* left_node) {
    if (end_slot > size_) end_slot = size_;
    std::memcpy(
        left_node->fv_.data() + left_node->offset(left_node->size_),
        fv_.data() + offset(0), end_slot * slot_size_
    );
    left_node->set_size(left_node->size_ + end_slot);
    shift(end_slot, - end_slot);
}

template class Node<int>;
template class Node<double>;
template class Node<Varchar>;

} // namespace minisql