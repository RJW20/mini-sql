#include "bplus_tree/leaf_node.hpp"

#include <utility>

#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"
#include "varchar.hpp"

namespace minisql {

/* Constructor for a new LeafNode.
 * Populates the pages header. */
LeafNode::LeafNode(
    FrameView&& fv, key_size_t key_size, slot_size_t slot_size,
    page_id_t parent, page_id_t next_leaf
) : Node(std::move(fv), Magic::LEAF_NODE, key_size, slot_size, parent) {
    set_next_leaf(next_leaf);
}

/* Transfer slots >= src's middle slot from src onto the front of dst.
 * Sets src's next_leaf to dst. */
void LeafNode::split(LeafNode* dst, LeafNode* src) {
    const size_t middle_slot = src->size_ / 2;
    splice_back_to_front(dst, src, src->size_ - middle_slot);
    src->set_next_leaf(dst->pid());
}

/* Transfer all slots from src onto the back of dst.
 * Sets dst's next_leaf to src's next_leaf. */
void LeafNode::merge(LeafNode* dst, LeafNode* src) {
    splice_front_to_back(dst, src, src->size_);
    dst->set_next_leaf(src->next_leaf());
}

} // namespace minisql