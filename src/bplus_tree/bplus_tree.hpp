#ifndef MINISQL_BPLUS_TREE_HPP
#define MINISQL_BPLUS_TREE_HPP

#include <cstddef>
#include <memory>

#include "bplus_tree/node.hpp"
#include "frame_manager/frame_manager.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "bplus_tree/leaf_node.hpp"
#include "span.hpp"
#include "bplus_tree/internal_node.hpp"

namespace minisql {

/* B+ Tree
 * Manages a tree structure of InternalNodes and LeafNodes throughout inserts
 * into and erases from LeafNodes for efficient key searching. */
class BPlusTree {
public:
    using key_size_t = Node::key_size_t;
    using slot_size_t = Node::slot_size_t;
    using size_t = Node::size_t;

    BPlusTree(
        FrameManager* fm, key_size_t key_size, slot_size_t slot_size,
        page_id_t root = nullpid
    );

    template <typename Key>
    static size_t seek_slot(Node* node, const Key& target);
    template <typename Key>
    std::unique_ptr<LeafNode> seek_leaf(const Key& target) const;

    template <typename Key>
    void insert_into(LeafNode* node, size_t slot, span<std::byte> bytes);
    template <typename Key>
    void erase_from(LeafNode* node, size_t slot);

    std::unique_ptr<LeafNode> open_leaf(page_id_t pid) const;

    page_id_t root() const noexcept { return root_; }

private:
    FrameManager* fm_;
    key_size_t key_size_;
    slot_size_t slot_size_;
    page_id_t root_;

    template <typename Key>
    void insert_into(
        std::unique_ptr<InternalNode> node, size_t slot, const Key& key,
        page_id_t pid
    );
    template <typename Key>
    void erase_from(std::unique_ptr<InternalNode> node, size_t slot);

    std::unique_ptr<InternalNode> open_internal(page_id_t pid) const;
    std::unique_ptr<Node> open_node(page_id_t pid) const;

    template <typename T>
    friend struct Wrapper;
};

} // namespace minisql

#endif // MINISQL_BPLUS_TREE_HPP