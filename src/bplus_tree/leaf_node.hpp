#ifndef MINISQL_LEAF_NODE_HPP
#define MINISQL_LEAF_NODE_HPP

#include <cstddef>
#include <utility>
#include <cstring>

#include "bplus_tree/node.hpp"
#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"
#include "span.hpp"

namespace minisql {

/* Leaf Node
 * A specialisation of Node in which slots store a raw span of bytes. */
template <typename Key>
class LeafNode : public Node<Key> {
public:
    using typename Node<Key>::key_size_t;
    using typename Node<Key>::slot_size_t;
    using typename Node<Key>::size_t;

    LeafNode(
        FrameView&& fv, key_size_t key_size, slot_size_t slot_size,
        page_id_t parent, page_id_t next_leaf = nullpid
    );
    LeafNode(FrameView&& fv) : Node<Key>{std::move(fv)} {}

    bool is_leaf() const override final { return true; }
    bool is_leftmost() const { return next_leaf() == nullpid; }

    page_id_t next_leaf() const {
        return this->fv_.template view<page_id_t>(
            LeafNodeHeader::NEXT_LEAF_OFFSET
        );
    }
    void set_next_leaf(page_id_t pid) {
        this->fv_.template write<page_id_t>(
            LeafNodeHeader::NEXT_LEAF_OFFSET, pid
        );
    }

    span<std::byte> slot(size_t slot) const {
        return span{this->fv_.data() + this->offset(slot), this->slot_size_};
    }
    void set_slot(size_t slot, span<std::byte> bytes) {
        std::memcpy(
            this->fv_.data() + this->offset(slot), bytes.data(),
            this->slot_size_
        );
    }

    void insert(size_t slot, span<std::byte> bytes) {
        this->shift(slot, 1);
        set_slot(slot, bytes);
    }

    Key split_into(LeafNode<Key>* node);
    Key take_back(LeafNode<Key>* node);
    Key take_front(LeafNode<Key>* node);
    void merge_into(LeafNode<Key>* node);

private:
    std::size_t header_size() const override final {
        return LeafNodeHeader::SIZE;
    }

    size_t min_size() const override final {
        return this->is_root() ? 0 : this->max_size() / 2;
    }
};

} // namespace minisql

#endif // MINISQL_LEAF_NODE_HPP