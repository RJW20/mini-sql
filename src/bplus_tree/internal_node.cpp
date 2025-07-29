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

/* Transfer slots > src's middle slot from src onto the front of dst and then
 * remove src's middle slot, with the key being returned and the page_id_t
 * being set as dst's first_child. */
template <typename Key>
Key InternalNode::split(InternalNode* dst, InternalNode* src) {
    const size_t middle_slot = src->size_ / 2 + src->size_ % 2 - 1;
    const Key separator = src->key<Key>(middle_slot);
    dst->set_first_child(src->child(middle_slot));
    splice_back_to_front(dst, src, src->size_ - (middle_slot + 1));
    src->erase(middle_slot);
    return separator;
}

/* Insert separator and src's first child at dst's last slot and then transfer
 * all slots from src onto the back of dst. */
template <typename Key>
void InternalNode::merge(
    InternalNode* dst, InternalNode* src, const Key& separator
) {
    dst->insert(dst->size_, separator, src->first_child());
    splice_front_to_back(dst, src, src->size_);
}

/* Insert separator and dst's first_child at dst's slot 0 and then remove the
 * last slot from src, with the key being returned and the page_id_t being set as
 * dst's first_child. */
template <typename Key>
Key InternalNode::take_back(
    InternalNode* dst, InternalNode* src, const Key& separator
) {
    dst->insert(0, separator, dst->first_child());
    const Key new_separator = src->key<Key>(src->size_ - 1);
    dst->set_first_child(src->child(src->size_ - 1));
    src->erase(src->size_ - 1);
    return new_separator;
}

/* Insert separator and src's first_child at dst's last slot and then remove
 * slot 0 from src, with the key being returned and the page_id_t being set as
 * src's first_child. */
template <typename Key>
Key InternalNode::take_front(
    InternalNode* dst, InternalNode* src, const Key& separator
) {
    dst->insert(dst->size_, separator, src->first_child());
    const Key new_separator = src->key<Key>(0);
    src->set_first_child(src->child(0));
    src->erase(0);
    return new_separator;
}

template int InternalNode::split<int>(InternalNode*, InternalNode*);
template double InternalNode::split<double>(InternalNode*, InternalNode*);
template Varchar InternalNode::split<Varchar>(InternalNode*, InternalNode*);

template void InternalNode::merge<int>(
    InternalNode*, InternalNode*, const int&
);
template void InternalNode::merge<double>(
    InternalNode*, InternalNode*, const double&
);
template void InternalNode::merge<Varchar>(
    InternalNode*, InternalNode*, const Varchar&
);

template int InternalNode::take_back<int>(
    InternalNode*, InternalNode*, const int&
);
template double InternalNode::take_back<double>(
    InternalNode*, InternalNode*, const double&
);
template Varchar InternalNode::take_back<Varchar>(
    InternalNode*, InternalNode*, const Varchar&
);

template int InternalNode::take_front<int>(
    InternalNode*, InternalNode*, const int&
);
template double InternalNode::take_front<double>(
    InternalNode*, InternalNode*, const double&
);
template Varchar InternalNode::take_front<Varchar>(
    InternalNode*, InternalNode*, const Varchar&
);

} // namespace minisql