#include "bplus_tree/leaf_node.hpp"

#include <utility>

#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"
#include "varchar.hpp"

namespace minisql {

/* Constructor for a new LeafNode.
 * Populates the pages header. */
template <typename Key>
LeafNode<Key>::LeafNode(
    FrameView&& fv, key_size_t key_size, slot_size_t slot_size,
    page_id_t parent, page_id_t next_leaf
) : Node<Key>(std::move(fv), Magic::LEAF_NODE, key_size, slot_size, parent) {
    set_next_leaf(next_leaf);
}

/* Transfer slots >= current middle slot onto the front of node.
 * Sets next_leaf to node.
 * Returns the key in the last slot. */
template <typename Key>
Key LeafNode<Key>::split_into(LeafNode<Key>* node) {
    const size_t middle_slot = this->size() / 2;
    this->transfer_to_front(node, middle_slot);
    set_next_leaf(node->pid());
    return this->key(middle_slot - 1);
}

/* Transfer node's last slot to slot 0.
 * Returns the key in node's last slot. */
template <typename Key>
Key LeafNode<Key>::take_back(LeafNode<Key>* node) {
    node->transfer_to_front(this, node->size() - 1);
    return node->key(node->size() - 1);
}

/* Transfer node's slot 0 to the last slot.
 * Return the key in the last slot. */
template <typename Key>
Key LeafNode<Key>::take_front(LeafNode<Key>* node) {
    node->transfer_to_back(this, 1);
    return this->key(this->size() - 1);
}

/* Transfer all slots onto the back of node.
 * Sets node's next_leaf to next_leaf. */
template <typename Key>
void LeafNode<Key>::merge_into(LeafNode<Key>* node) {
    this->transfer_to_back(node, this->size());
    node->set_next_leaf(next_leaf());
}

template class LeafNode<int>;
template class LeafNode<double>;
template class LeafNode<Varchar>;

} // namespace minisql