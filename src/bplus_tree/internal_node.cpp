#include "bplus_tree/internal_node.hpp"

#include <utility>

#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"
#include "varchar.hpp"

namespace minisql {

/* Constructor for a new InternalNode.
 * Populates the pages header. */
InternalNode::InternalNode(
    FrameView&& fv, key_size_t key_size, page_id_t parent,
    page_id_t first_child
) : Node(
    std::move(fv), Magic::INTERNAL_NODE, key_size,
    key_size + sizeof(page_id_t), parent
) {
    set_first_child(first_child);
}

/* Transfer slots > current middle slot onto the front of node and then remove
 * the current middle slot, with the key being returned and the page_id_t being
 * set as node's first_child. */
template <typename Key>
Key InternalNode::split_into(InternalNode* node) {
    const size_t middle_slot = size() / 2 + size() % 2 - 1;
    const Key separator = key<Key>(middle_slot);
    node->set_first_child(child(middle_slot));
    transfer_to_front(node, size() - (middle_slot + 1));
    remove(middle_slot);
    return separator;
}

/* Insert separator and first_child at slot 0 and then remove the last slot
 * from node, with the key being returned and the page_id_t being set as
 * first_child. */
template <typename Key>
Key InternalNode::take_back(InternalNode* node, const Key& separator) {
    insert(0, separator, first_child());
    const Key new_separator = node->key<Key>(node->size() - 1);
    set_first_child(node->child(node->size() - 1));
    node->remove(node->size() - 1);
    return new_separator;
}

/* Insert separator and node's first_child at the last slot and then remove
 * slot 0 from node, with the key being returned and the page_id_t being set as
 * node's first_child. */
template <typename Key>
Key InternalNode::take_front(InternalNode* node, const Key& separator) {
    insert(size(), separator, node->first_child());
    const Key new_separator = node->key<Key>(0);
    node->set_first_child(node->child(0));
    node->remove(0);
    return new_separator;
}

/* Insert separator and first child at node's last slot and then transfer all
 * slots onto the back of node. */
template <typename Key>
void InternalNode::merge_into(InternalNode* node, const Key& separator) {
    node->insert(node->size(), separator, first_child());
    transfer_to_back(node, size());
}

template int InternalNode::split_into<int>(InternalNode* node);
template double InternalNode::split_into<double>(InternalNode* node);
template Varchar InternalNode::split_into<Varchar>(InternalNode* node);

template int InternalNode::take_back<int>(
    InternalNode* node, const int& separator
);
template double InternalNode::take_back<double>(
    InternalNode* node, const double& separator
);
template Varchar InternalNode::take_back<Varchar>(
    InternalNode* node, const Varchar& separator
);

template int InternalNode::take_front<int>(
    InternalNode* node, const int& separator
);
template double InternalNode::take_front<double>(
    InternalNode* node, const double& separator
);
template Varchar InternalNode::take_front<Varchar>(
    InternalNode* node, const Varchar& separator
);

template void InternalNode::merge_into<int>(
    InternalNode* node, const int& separator
);
template void InternalNode::merge_into<double>(
    InternalNode* node, const double& separator
);
template void InternalNode::merge_into<Varchar>(
    InternalNode* node, const Varchar& separator
);

} // namespace minisql