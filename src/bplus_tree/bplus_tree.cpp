#include "bplus_tree/bplus_tree.hpp"

#include <cstddef>
#include <memory>
#include <utility>

#include "bplus_tree/internal_node.hpp"
#include "bplus_tree/leaf_node.hpp"
#include "bplus_tree/node.hpp"
#include "exceptions/engine_exceptions.hpp"
#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "frame_manager/frame_manager.hpp"
#include "headers.hpp"
#include "span.hpp"

namespace minisql {

/* Constructor for B+ Tree.
 * Creates a new root LeafNode if necessary. */
BPlusTree::BPlusTree(
    FrameManager* fm, key_size_t key_size, slot_size_t slot_size,
    page_id_t root
) : fm_{fm}, key_size_{key_size}, slot_size_{slot_size}, root_{root} {
    if (root_ == nullpid) {
        LeafNode root_node(fm_->allocate(), key_size_, slot_size_, nullpid);
        root_ = root_node.pid();
    }
}

/* Return the first slot in node containing a key >= target.
 * Returns node.size() if target > all keys in node.
 * Assumes the slots are ordered by key and applies binary search. */
template <typename Key>
Node::size_t BPlusTree::seek_slot(Node* node, const Key& target) {
    size_t l = 0;
    size_t r = node->size();
    size_t m;
    while (l < r) {
        m = (l + r) / 2;
        if (node->key<Key>(m) < target) l = m + 1;
        else r = m;
    }
    return r;
}

// Return the LeafNode that target falls within (if it exists in this B+ Tree).
template <typename Key>
std::unique_ptr<LeafNode> BPlusTree::seek_leaf(const Key& target) const {
    std::unique_ptr<Node> current = open_node(root_);
    while(!current->is_leaf()) current = open_node(
        dynamic_cast<InternalNode*>(current.get())->child(
            seek_slot<Key>(current.get(), target) - 1
        )
    );
    return std::unique_ptr<LeafNode>(dynamic_cast<LeafNode*>(
        current.release()
    ));
}

/* Copy bytes' underlying data to the given slot in node.
 * Shifts all slots >= slot to the right by 1.
 * If node has the maximum number of slots and requires splitting the tree
 * above will be adjusted accordingly. */
template <typename Key>
void BPlusTree::insert_into(LeafNode* node, size_t slot, span<std::byte> bytes)
{
    // Attempt to insert into node
    if (!node->at_max_capacity()) {
        node->insert(slot, bytes);
        return;
    }

    // Carry out a split
    LeafNode new_node{
        fm_->allocate(), key_size_, slot_size_, node->parent(),
        node->next_leaf()
    };
    LeafNode::split(&new_node, node);
    if (slot <= node->size()) node->insert(slot, bytes);
    else new_node.insert(slot - node->size(), bytes);

    // If node is the root then create a parent
    if (node->is_root()) {
        InternalNode root{fm_->allocate(), key_size_, nullpid, root_};
        root_ = root.pid();
        node->set_parent(root_);
        new_node.set_parent(root_);
    }

    // Insert new_node into parent
    std::unique_ptr<InternalNode> parent = open_internal(node->parent());
    const Key separator = node->key<Key>(node->size() - 1);
    slot = seek_slot<Key>(parent.get(), separator);
    insert_into<Key>(std::move(parent), slot, separator, new_node.pid());
}

/* Remove the given slot from node.
 * If node has the minimum number of slots and requires merging the tree above
 * above will be adjusted accordingly. */
template <typename Key>
void BPlusTree::erase_from(LeafNode* node, size_t slot) {

    // Attempt to erase from node
    if (!node->at_min_capacity()) {
        node->erase(slot);
        return;
    }

    // If node is at min_capacity and root then there is nothing to erase
    if (node->is_root()) return;

    // Get parent and position within it
    std::unique_ptr<InternalNode> parent = open_internal(node->parent());
    size_t child_slot = seek_slot<Key>(
        parent.get(), node->key<Key>(node->size() - 1)
    ) - 1;

    // Try to take from a sibling
    if (child_slot != static_cast<size_t>(-1)) {
        std::unique_ptr<LeafNode> sibling = open_leaf(
            parent->child(child_slot - 1)
        );
        if (!sibling->at_min_capacity()) {
            LeafNode::take_back(node, sibling.get());
            parent->set_key<Key>(
                child_slot, sibling->key<Key>(sibling->size() - 1)
            );
            node->erase(slot + 1);
            return;
        }
    }
    if (child_slot != parent->size() - 1) {
        std::unique_ptr<LeafNode> sibling = open_leaf(
            parent->child(child_slot + 1)
        );
        if (!sibling->at_min_capacity()) {
            LeafNode::take_front(node, sibling.get());
            parent->set_key<Key>(
                child_slot + 1, node->key<Key>(node->size() - 1)
            );
            node->erase(slot);
            return;
        }
    }

    // Merge with a sibling
    if (child_slot != static_cast<size_t>(-1)) {
        std::unique_ptr<LeafNode> sibling = open_leaf(
            parent->child(child_slot - 1)
        );
        slot = sibling->size() + slot;
        LeafNode::merge(sibling.get(), node);
        sibling->erase(slot);
        erase_from<Key>(std::move(parent), child_slot);
        fm_->deallocate(node->pid());
    }
    else {
        std::unique_ptr<LeafNode> sibling = open_leaf(parent->child(0));
        LeafNode::merge(node, sibling.get());
        node->erase(slot);
        erase_from<Key>(std::move(parent), 0);
        fm_->deallocate(sibling->pid());
    }
}

/* Copy the given key and pid to the given slot in node.
 * Shifts all slots >= slot to the right by 1.
 * If node has the maximum number of slots and requires splitting the tree
 * above will be adjusted accordingly. */
template <typename Key>
void BPlusTree::insert_into(
    std::unique_ptr<InternalNode> node, size_t slot, const Key& key,
    page_id_t pid
) {
    // Attempt to insert into node
    if (!node->at_max_capacity()) {
        node->insert<Key>(slot, key, pid);
        return;
    }

    // Carry out a split
    InternalNode new_node{fm_->allocate(), key_size_, node->parent()};
    const Key separator = InternalNode::split<Key>(&new_node, node.get());
    if (slot <= node->size()) node->insert<Key>(slot, key, pid);
    else new_node.insert<Key>(slot - node->size() - 1, key, pid);

    // Update parent for moved children
    const page_id_t new_parent = new_node.pid();
    for (int i = -1; i < new_node.size(); i++)
        open_node(new_node.child(i))->set_parent(new_parent);

    // If node is the root then create a parent
    if (node->is_root()) {
        InternalNode root{fm_->allocate(), key_size_, nullpid, root_};
        root_ = root.pid();
        node->set_parent(root_);
        new_node.set_parent(root_);
    }

    // Insert new_node into parent
    std::unique_ptr<InternalNode> parent = open_internal(node->parent());
    slot = seek_slot<Key>(parent.get(), separator);
    insert_into<Key>(std::move(parent), slot, separator, new_parent);
}

/* Remove the given slot from node.
 * If node has the minimum number of slots and requires merging the tree above
 * above will be adjusted accordingly. */
template <typename Key>
void BPlusTree::erase_from(std::unique_ptr<InternalNode> node, size_t slot) {

    // Attempt to erase from node
    if (!node->at_min_capacity()) {
        node->erase(slot);
        return;
    }

    // If node is at min_capacity and root then erase it
    if (node->is_root()) {
        root_ = node->child(-1);
        fm_->deallocate(node->pid());
        open_node(root_)->set_parent(nullpid);
        return;
    }

    // Get parent and position within it
    std::unique_ptr<InternalNode> parent = open_internal(node->parent());
    size_t child_slot = seek_slot<Key>(
        parent.get(), node->key<Key>(node->size() - 1)
    ) - 1;

    // Try to take from a sibling
    if (child_slot != static_cast<size_t>(-1)) {
        std::unique_ptr<InternalNode> sibling = open_internal(
            parent->child(child_slot - 1)
        );
        if (!sibling->at_min_capacity()) {
            const Key separator = InternalNode::take_back<Key>(
                node.get(), sibling.get(), parent->key<Key>(child_slot)
            );
            parent->set_key<Key>(child_slot, separator);
            open_node(node->child(-1))->set_parent(node->pid());
            node->erase(slot + 1);
            return;
        }
    }
    if (child_slot != parent->size() - 1) {
        std::unique_ptr<InternalNode> sibling = open_internal(
            parent->child(child_slot + 1)
        );
        if (!sibling->at_min_capacity()) {
            const Key separator = InternalNode::take_front<Key>(
                node.get(), sibling.get(), parent->key<Key>(child_slot + 1)
            );
            parent->set_key<Key>(child_slot + 1, separator);
            open_node(node->child(node->size() - 1))->set_parent(node->pid());
            node->erase(slot);
            return;
        }
    }

    // Merge with a sibling
    if (child_slot != static_cast<size_t>(-1)) {
        std::unique_ptr<InternalNode> sibling = open_internal(
            parent->child(child_slot - 1)
        );
        const page_id_t new_parent = sibling->pid();
        for (int i = -1; i < node->size(); i++)
            open_node(node->child(i))->set_parent(new_parent);
        slot = sibling->size() + 1 + slot;
        InternalNode::merge<Key>(
            sibling.get(), node.get(), parent->key<Key>(child_slot)
        );
        sibling->erase(slot);
        erase_from<Key>(std::move(parent), child_slot);
        fm_->deallocate(node->pid());
    }
    else {
        std::unique_ptr<InternalNode> sibling = open_internal(parent->child(0));
        const page_id_t new_parent = node->pid();
        for (int i = -1; i < sibling->size(); i++)
            open_node(sibling->child(i))->set_parent(new_parent);
        InternalNode::merge<Key>(
            node.get(), sibling.get(), parent->key<Key>(0)
        );
        node->erase(slot);
        erase_from<Key>(std::move(parent), 0);
        fm_->deallocate(sibling->pid());
    }
}

/* Return the LeafNode corresponding to the given page_id_t.
 * Throws a MagicException if the page corresponding to the page_id_t does not
 * have a LEAF_NODE magic. */
std::unique_ptr<LeafNode> BPlusTree::open_leaf(page_id_t pid) const {
    FrameView fv = fm_->pin(pid);
    Magic magic = fv.view<Magic>(NodeHeader::MAGIC_OFFSET);
    if (magic == Magic::LEAF_NODE)
        return std::make_unique<LeafNode>(std::move(fv));
    throw MagicException(magic);
}

/* Return the InternalNode corresponding to the given page_id_t.
 * Throws a MagicException if the page corresponding to the page_id_t does not
 * have an INTERNAL_NODE magic. */
std::unique_ptr<InternalNode> BPlusTree::open_internal(page_id_t pid) const {
    FrameView fv = fm_->pin(pid);
    Magic magic = fv.view<Magic>(NodeHeader::MAGIC_OFFSET);
    if (magic == Magic::INTERNAL_NODE)
        return std::make_unique<InternalNode>(std::move(fv));
    throw MagicException(magic);
}

/* Return the Node corresponding to the given page_id_t.
 * To be used when the type of Node pointed to by the page_id_t is unknown.
 * Throws a MagicException if the page corresponding to the page_id_t does not
 * have a valid node magic. */
std::unique_ptr<Node> BPlusTree::open_node(page_id_t pid) const {
    FrameView fv = fm_->pin(pid);
    Magic magic = fv.view<Magic>(NodeHeader::MAGIC_OFFSET);
    switch (magic) {
        case Magic::INTERNAL_NODE:
            return std::make_unique<InternalNode>(std::move(fv));
        case Magic::LEAF_NODE:
            return std::make_unique<LeafNode>(std::move(fv));
        default:
            throw MagicException(magic);
    }
}

// Destroy node and the entire sub-tree it contains.
void BPlusTree::destroy(std::unique_ptr<Node> node) {
    if (!node->is_leaf()) {
        InternalNode* internal = dynamic_cast<InternalNode*>(node.get());
        destroy(open_node(internal->child(-1)));
        for (size_t slot = 0; slot < internal->size(); slot++)
            destroy(open_node(internal->child(slot)));
    }
    fm_->deallocate(node->pid());
}

// Explicitly instantiate templated methods for all Field types.
#define FIELD_TYPE(T)                                                         \
    template BPlusTree::size_t BPlusTree::seek_slot<T>(Node*, const T&);      \
    template std::unique_ptr<LeafNode> BPlusTree::seek_leaf<T>(               \
        const T&                                                              \
    ) const;                                                                  \
    template void BPlusTree::insert_into<T>(                                  \
        LeafNode*, size_t, span<std::byte> bytes                              \
    );                                                                        \
    template void BPlusTree::erase_from<T>(LeafNode*, size_t);                \
    template void BPlusTree::insert_into<T>(                                  \
        std::unique_ptr<InternalNode>, size_t, const T&, page_id_t            \
    );                                                                        \
    template void BPlusTree::erase_from<T>(                                   \
        std::unique_ptr<InternalNode>, size_t                                 \
    );
#define FIELD_TYPE_LAST(T) FIELD_TYPE(T)

#include "minisql/field_types.def"
#undef FIELD_TYPE
#undef FIELD_TYPE_LAST

} // namespace minisql