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

/* Transfer slots >= current middle slot onto the front of node.
 * Sets next_leaf to node. */
void LeafNode::split_into(LeafNode* node) {
    const size_t middle_slot = size() / 2;
    transfer_to_front(node, size() - middle_slot);
    set_next_leaf(node->pid());
}

/* Transfer all slots onto the back of node.
 * Sets node's next_leaf to next_leaf. */
void LeafNode::merge_into(LeafNode* node) {
    transfer_to_back(node, size());
    node->set_next_leaf(next_leaf());
}

} // namespace minisql