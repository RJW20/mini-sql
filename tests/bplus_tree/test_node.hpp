#ifndef TEST_NODE_HPP
#define TEST_NODE_HPP

#include "bplus_tree/node.hpp"

#include <cstddef>
#include <utility>

#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"

using namespace minisql;

// A Node with each key taking up the entirety of a slot
class TestNode : public Node {
public:
    TestNode(FrameView&& fv, key_size_t key_size, page_id_t parent)
        : Node{std::move(fv), Magic{0xFF}, key_size, key_size, parent} {}
    using Node::Node;

    bool is_leaf() const override final { return false; }

    template <typename Key>
    void insert(size_t slot, const Key& key) {
        shift(slot, 1);
        set_key(slot, key);
    }

    using Node::shift;

    using Node::assert_compatibility;
    using Node::splice_back_to_front;
    using Node::splice_front_to_back;

private:
    std::size_t header_size() const override final {
        return NodeHeader::SIZE;
    }

    size_t min_size() const override final { return 0; }
};

#endif // TEST_NODE_HPP