#ifndef MINISQL_INTERNAL_NODE_HPP
#define MINISQL_INTERNAL_NODE_HPP

#include <cstddef>
#include <utility>

#include "bplus_tree/node.hpp"
#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"

namespace minisql {

/* Internal Node
 * A speciailisation of Node in which slots are comprised of a key and a
 * page_id_t. */
template <typename Key>
class InternalNode : public Node<Key> {
public:
    using typename Node<Key>::key_size_t;
    using typename Node<Key>::size_t;

    InternalNode(
        FrameView&& fv, key_size_t key_size, page_id_t parent,
        page_id_t first_child = nullpid
    );
    InternalNode(FrameView&& fv) : Node<Key>(std::move(fv)) {}

    bool is_leaf() const override { return false; }

    page_id_t child(size_t slot) const {
        if (slot == -1) return first_child();
        return this->fv_.template view<page_id_t>(
            this->offset(slot) + this->key_size_);
    }
    void set_child(size_t slot, page_id_t pid) {
        if (slot == -1) set_first_child(pid);
        else this->fv_.template write<page_id_t>(
            this->offset(slot) + this->key_size_, pid
        );
    }

    void insert(size_t slot, const Key& key, page_id_t pid) {
        this->shift(slot, 1);
        this->set_key(slot, key);
        set_child(slot, pid);
    }

    Key split_into(InternalNode<Key>* node);
    Key take_back(InternalNode<Key>* node, const Key& separator);
    Key take_front(InternalNode<Key>* node, const Key& separator);
    void merge_into(InternalNode<Key>* node, const Key& separator);

private:
    page_id_t first_child() const {
        return this->fv_.template view<page_id_t>(
            InternalNodeHeader::FIRST_CHILD_OFFSET
        );
    }
    void set_first_child(page_id_t pid) {
        this->fv_.template write<page_id_t>(
            InternalNodeHeader::FIRST_CHILD_OFFSET, pid
        );
    }

    std::size_t header_size() const override {
        return InternalNodeHeader::SIZE;
    }

    size_t min_size() const override {
        if (this->is_root()) return 1;
        size_t max_size_ = this->max_size();
        return max_size_ / 2 + max_size_ % 2 - 1;
    }
};

} // namespace minisql

#endif // MINISQL_INTERNAL_NODE_HPP