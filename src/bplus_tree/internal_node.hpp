#ifndef MINISQL_INTERNAL_NODE_HPP
#define MINISQL_INTERNAL_NODE_HPP

#include <cstddef>
#include <utility>

#include "bplus_tree/node.hpp"
#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"

namespace minisql {

/* Internal Node
 * A speciailisation of Node in which slots are comprised of a key and a
 * page_id_t. */
class InternalNode : public Node {
public:
    InternalNode(
        FrameView&& fv, key_size_t key_size, page_id_t parent,
        page_id_t first_child = nullpid
    );
    InternalNode(FrameView&& fv) : Node{std::move(fv)} {}

    bool is_leaf() const override final { return false; }

    page_id_t child(size_t slot) const {
        if (slot == -1) return first_child();
        return fv_.view<page_id_t>(offset(slot) + key_size_);
    }
    void set_child(size_t slot, page_id_t pid) {
        if (slot == -1) set_first_child(pid);
        else fv_.write<page_id_t>(offset(slot) + key_size_, pid);
    }

    template <typename Key>
    void insert(size_t slot, const Key& key, page_id_t pid) {
        shift(slot, 1);
        set_key(slot, key);
        set_child(slot, pid);
    }

    template <typename Key>
    Key split_into(InternalNode* node);
    template <typename Key>
    Key take_back(InternalNode* node, const Key& separator);
    template <typename Key>
    Key take_front(InternalNode* node, const Key& separator);
    template <typename Key>
    void merge_into(InternalNode* node, const Key& separator);

private:
    page_id_t first_child() const {
        return fv_.view<page_id_t>(InternalNodeHeader::FIRST_CHILD_OFFSET);
    }
    void set_first_child(page_id_t pid) {
        fv_.write<page_id_t>(InternalNodeHeader::FIRST_CHILD_OFFSET, pid);
    }

    std::size_t header_size() const override final {
        return InternalNodeHeader::SIZE;
    }

    size_t min_size() const override final {
        if (is_root()) return 1;
        size_t max_size_ = max_size();
        return max_size_ / 2 + max_size_ % 2 - 1;
    }
};

} // namespace minisql

#endif // MINISQL_INTERNAL_NODE_HPP