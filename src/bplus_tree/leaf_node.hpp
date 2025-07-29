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
class LeafNode : public Node {
public:
    LeafNode(
        FrameView&& fv, key_size_t key_size, slot_size_t slot_size,
        page_id_t parent, page_id_t next_leaf = nullpid
    );
    LeafNode(FrameView&& fv) : Node{std::move(fv)} {}

    bool is_leaf() const override final { return true; }
    bool is_leftmost() const { return next_leaf() == nullpid; }

    page_id_t next_leaf() const {
        return fv_.view<page_id_t>(LeafNodeHeader::NEXT_LEAF_OFFSET);
    }
    void set_next_leaf(page_id_t pid) {
        fv_.write<page_id_t>(LeafNodeHeader::NEXT_LEAF_OFFSET, pid);
    }

    span<std::byte> slot(size_t slot) const {
        return span{fv_.data() + offset(slot), slot_size_};
    }
    void set_slot(size_t slot, span<std::byte> bytes) {
        std::memcpy(fv_.data() + offset(slot), bytes.data(), slot_size_);
    }

    void insert(size_t slot, span<std::byte> bytes) {
        shift(slot, 1);
        set_slot(slot, bytes);
    }

    static void split(LeafNode* dst, LeafNode* src);
    static void merge(LeafNode* dst, LeafNode* src);
    
    static void take_back(LeafNode* dst, LeafNode* src) {
        splice_back_to_front(dst, src, 1);
    }
    static void take_front(LeafNode* dst, LeafNode* src) {
        splice_front_to_back(dst, src, 1);
    }

private:
    std::size_t header_size() const override final {
        return LeafNodeHeader::SIZE;
    }

    size_t min_size() const override final {
        return is_root() ? 0 : max_size() / 2;
    }
};

} // namespace minisql

#endif // MINISQL_LEAF_NODE_HPP