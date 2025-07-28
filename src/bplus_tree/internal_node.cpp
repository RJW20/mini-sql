#include "bplus_tree/internal_node.hpp"

#include <utility>

#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"
#include "varchar.hpp"

namespace minisql {

/* Constructor for a new InternalNode.
 * Populates the pages header. */
template <typename Key>
InternalNode<Key>::InternalNode(
    FrameView&& fv, key_size_t key_size, page_id_t parent,
    page_id_t first_child
) : Node<Key>(
    std::move(fv), Magic::INTERNAL_NODE, key_size,
    key_size + sizeof(page_id_t), parent
) {
    set_first_child(first_child);
}

/* Transfer slots > current middle slot onto the front of node and then remove
 * the current middle slot, with the key being returned and the page_id_t being
 * set as node's first_child. */
template <typename Key>
Key InternalNode<Key>::split_into(InternalNode<Key>* node) {
    const size_t middle_slot = this->size() / 2 + this->size() % 2 - 1;
    const Key separator = this->key(middle_slot);
    node->set_first_child(child(middle_slot));
    this->transfer_to_front(node, middle_slot + 1);
    this->remove(middle_slot);
    return separator;
}

/* Insert separator and first_child at slot 0 and then remove the last slot
 * from node, with the key being returned and the page_id_t being set as
 * first_child. */
template <typename Key>
Key InternalNode<Key>::take_back(
    InternalNode<Key>* node, const Key& separator
) {
    insert(0, separator, first_child());
    const Key new_separator = node->key(node->size() - 1);
    set_first_child(node->child(node->size() - 1));
    node->remove(node->size() - 1);
    return new_separator;
}

/* Insert separator and node's first_child at the last slot and then remove
 * slot 0 from node, with the key being returned and the page_id_t being set as
 * node's first_child. */
template <typename Key>
Key InternalNode<Key>::take_front(
    InternalNode<Key>* node, const Key& separator
) {
    insert(this->size(), separator, node->first_child());
    const Key new_separator = node->key(0);
    node->set_first_child(node->child(0));
    node->remove(0);
    return new_separator;
}

/* Insert separator and first child at node's last slot and then transfer all
 * slots onto the back of node. */
template <typename Key>
void InternalNode<Key>::merge_into(
    InternalNode<Key>* node, const Key& separator
) {
    node->insert(node->size(), separator, first_child());
    this->transfer_to_back(node, this->size());
}

template class InternalNode<int>;
template class InternalNode<double>;
template class InternalNode<Varchar>;

} // namespace minisql