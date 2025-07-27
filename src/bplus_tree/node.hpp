#ifndef MINISQL_NODE_HPP
#define MINISQL_NODE_HPP

#include <cstddef>

#include "bplus_tree/keys.hpp"
#include "headers.hpp"
#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"

namespace minisql {

/* Node
 * Acts as an interface over a page, consisting of a NodeHeader followed by an
 * array of fixed-size slots of bytes. */
template <typename Key>
class Node {
    static_assert(IsValidKey<Key>::value, "Key type not supported.");

public:
    using key_size_t = NodeHeader::key_size_t;
    using slot_size_t = NodeHeader::slot_size_t;
    using size_t = NodeHeader::size_t;

    Node(
        FrameView&& fv, Magic magic, key_size_t key_size,
        slot_size_t slot_size, page_id_t parent
    );
    Node(FrameView&& fv);
    virtual ~Node() = default;

    bool is_root() const { return parent() == nullpid; }
    virtual bool is_leaf() const = 0;

    page_id_t pid() const { return fv_.pid(); }

    page_id_t parent() const {
        return fv_.view<page_id_t>(NodeHeader::PARENT_OFFSET);
    }
    void set_parent(page_id_t pid) {
        fv_.write<page_id_t>(NodeHeader::PARENT_OFFSET, pid);
    }

    Key key(size_t slot) const {
        return fv_.view<Key>(offset(slot), key_size_);
    }
    void set_key(size_t slot, const Key& key) {
        fv_.write<Key>(offset(slot), key);
    }

    void remove(size_t slot) { shift(slot + 1, -1); }

    size_t size() const { return size_; }
    bool at_min_capacity() const { return size_ == min_size(); }
    bool at_max_capacity() const { return size_ == max_size(); }

protected:
    FrameView fv_;
    key_size_t key_size_;
    slot_size_t slot_size_;
    size_t size_;

    std::size_t offset(size_t slot) const { 
        return header_length() + slot * slot_size_;
    }

    void shift(size_t start_slot, int steps);
    void transfer_right(size_t start_slot, Node<Key>* right_node);
    void transfer_left(size_t end_slot, Node<Key>* left_node);

private:
    virtual std::size_t header_length() const = 0;

    void set_size(size_t size) {
        size_ = size;
        fv_.write<size_t>(NodeHeader::SIZE_OFFSET, size_);
    }

    virtual size_t min_size() const = 0;
    size_t max_size() const {
        return (fv_.page_size() - header_length()) / slot_size_;
    }
};

} // namespace minisql

#endif // MINISQL_NODE_HPP