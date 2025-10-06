#ifndef TEST_NODE_HPP
#define TEST_NODE_HPP

#include <cstddef>
#include <utility>

#include "bplus_tree/node.hpp"
#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"

// A Node with each key taking up the entirety of a slot.
class TestNode : public minisql::Node {
public:
    TestNode(
        minisql::FrameView&& fv, key_size_t key_size, minisql::page_id_t parent
    ) : minisql::Node{
            std::move(fv), minisql::Magic{0xFF}, key_size, key_size, parent
        } {}
    using minisql::Node::Node;

    bool is_leaf() const override final { return false; }

    template <typename Key>
    void insert(size_t slot, const Key& key) {
        shift(slot, 1);
        set_key(slot, key);
    }

    using minisql::Node::shift;

    using minisql::Node::assert_compatibility;
    using minisql::Node::splice_back_to_front;
    using minisql::Node::splice_front_to_back;

private:
    std::size_t header_size() const override final {
        return minisql::NodeHeader::SIZE;
    }

    size_t min_size() const override final { return 0; }
};

#endif // TEST_NODE_HPP