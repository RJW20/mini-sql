#include "bplus_tree/leaf_node.hpp"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

#include "bplus_tree/node.hpp"
#include "frame_manager/cache/frame.hpp"
#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"
#include "span.hpp"

#include "utils.hpp"

using namespace minisql;

void test_new_constructor() {
    Frame f;
    f.data.resize(2048);
    const page_id_t next_leaf = generate<page_id_t>();
    LeafNode node{
        FrameView{nullptr, &f}, 0, 100, generate<page_id_t>(), next_leaf
    };
    assert(node.is_leaf());
    assert(node.is_rightmost() == (next_leaf == nullpid));
    assert(node.next_leaf() == next_leaf);
    std::cout << "- test_new_constructor passed" << std::endl;
}

void test_insert() {
    Frame f;
    f.data.resize(2048);
    const Node::slot_size_t slot_size = 100;
    const Node::size_t max_slots =
        (f.data.size() - LeafNodeHeader::SIZE) / slot_size;
    LeafNode node{
        FrameView{nullptr, &f}, 0, slot_size, generate<page_id_t>()
    };
    for (int i = 0; i < max_slots; i++) {
        std::vector<std::byte> bytes(slot_size, static_cast<std::byte>(i));
        node.insert(i, bytes);
        assert(node.size() == i + 1);
        span<std::byte> slot = node.slot(i);
        assert(slot.size() == bytes.size());
        assert(slot[0] == bytes[0]);
    }
    assert(node.at_max_capacity());
    std::cout << "- test_insert passed" << std::endl;
}

void test_split() {
    Frame f1, f2;
    const std::size_t page_size = 2048;
    f1.data.resize(page_size);
    f2.data.resize(page_size);
    const Node::slot_size_t slot_size = 100;
    const Node::size_t max_slots =
        (page_size - LeafNodeHeader::SIZE) / slot_size;

    LeafNode dst{
        FrameView{nullptr, &f1}, 0, slot_size, generate<page_id_t>()
    };
    LeafNode src{
        FrameView{nullptr, &f2}, 0, slot_size, generate<page_id_t>()
    };

    for (int i = 0; i < max_slots; i++) {
        std::vector<std::byte> bytes(slot_size, static_cast<std::byte>(i));
        src.insert(i, bytes);
    }

    const Node::size_t middle_slot = max_slots / 2;
    LeafNode::split(&dst, &src);
    assert(dst.size() == max_slots - middle_slot);
    for (int i = 0; i < dst.size(); i++) {
        span<std::byte> slot = dst.slot(i);
        assert(slot.size() == slot_size);
        assert(slot[0] == static_cast<std::byte>(i + middle_slot));
    }
    assert(src.size() == middle_slot);
    for (int i = 0; i < src.size(); i++) {
        span<std::byte> slot = src.slot(i);
        assert(slot.size() == slot_size);
        assert(slot[0] == static_cast<std::byte>(i));
    }

    std::cout << "- test_split passed" << std::endl;
}

void test_merge() {
    Frame f1, f2;
    const std::size_t page_size = 2048;
    f1.data.resize(page_size);
    f2.data.resize(page_size);
    const Node::slot_size_t slot_size = 100;
    const Node::size_t max_slots =
        (page_size - LeafNodeHeader::SIZE) / slot_size;
    const Node::size_t min_slots = max_slots / 2;;

    LeafNode dst{
        FrameView{nullptr, &f1}, 0, slot_size, generate<page_id_t>()
    };
    LeafNode src{
        FrameView{nullptr, &f2}, 0, slot_size, generate<page_id_t>()
    };

    for (int i = 0; i < min_slots; i++) {
        std::vector<std::byte> bytes(slot_size, static_cast<std::byte>(i));
        dst.insert(i, bytes);
        src.insert(i, bytes);
    }

    LeafNode::merge(&dst, &src);
    assert(dst.size() == 2 * min_slots);
    for (int i = 0; i < min_slots; i++) {
        span<std::byte> slot = dst.slot(i);
        assert(slot.size() == slot_size);
        assert(slot[0] == static_cast<std::byte>(i));
    }
    for (int i = min_slots; i < dst.size(); i++) {
        span<std::byte> slot = dst.slot(i);
        assert(slot.size() == slot_size);
        assert(slot[0] == static_cast<std::byte>(i - min_slots));
    }
    assert(!src.size());

    std::cout << "- test_merge passed" << std::endl;
}

void test_take() {
    Frame f1, f2;
    const std::size_t page_size = 2048;
    f1.data.resize(page_size);
    f2.data.resize(page_size);
    const Node::slot_size_t slot_size = 100;
    const Node::size_t max_slots =
        (page_size - LeafNodeHeader::SIZE) / slot_size;
    {
        LeafNode dst{
            FrameView{nullptr, &f1}, 0, slot_size, generate<page_id_t>()
        };
        LeafNode src{
            FrameView{nullptr, &f2}, 0, slot_size, generate<page_id_t>()
        };

        for (int i = 0; i < max_slots - 1; i++) {
            std::vector<std::byte> bytes(slot_size, static_cast<std::byte>(i));
            dst.insert(i, bytes);
        }
        for (int i = 0; i < max_slots; i++) {
            std::vector<std::byte> bytes(slot_size, static_cast<std::byte>(i));
            src.insert(i, bytes);
        }

        LeafNode::take_back(&dst, &src);
        assert(dst.size() == max_slots);
        span<std::byte> slot_0 = dst.slot(0);
        assert(slot_0.size() == slot_size);
        assert(slot_0[0] == static_cast<std::byte>(max_slots - 1));
        for (int i = 1; i < dst.size(); i++) {
            span<std::byte> slot = dst.slot(i);
            assert(slot.size() == slot_size);
            assert(slot[0] == static_cast<std::byte>(i - 1));
        }
        assert(src.size() == max_slots - 1);
        for (int i = 0; i < src.size(); i++) {
            span<std::byte> slot = src.slot(i);
            assert(slot.size() == slot_size);
            assert(slot[0] == static_cast<std::byte>(i));
        }
    }
    {
        LeafNode dst{
            FrameView{nullptr, &f1}, 0, slot_size, generate<page_id_t>()
        };
        LeafNode src{
            FrameView{nullptr, &f2}, 0, slot_size, generate<page_id_t>()
        };

        for (int i = 0; i < max_slots - 1; i++) {
            std::vector<std::byte> bytes(slot_size, static_cast<std::byte>(i));
            dst.insert(i, bytes);
        }
        for (int i = 0; i < max_slots; i++) {
            std::vector<std::byte> bytes(slot_size, static_cast<std::byte>(i));
            src.insert(i, bytes);
        }

        LeafNode::take_front(&dst, &src);
        assert(dst.size() == max_slots);
        for (int i = 0; i < dst.size() - 1; i++) {
            span<std::byte> slot = dst.slot(i);
            assert(slot.size() == slot_size);
            assert(slot[0] == static_cast<std::byte>(i));
        }
        span<std::byte> slot_end = dst.slot(dst.size() - 1);
        assert(slot_end.size() == slot_size);
        assert(slot_end[0] == static_cast<std::byte>(0));
        assert(src.size() == max_slots - 1);
        for (int i = 0; i < src.size(); i++) {
            span<std::byte> slot = src.slot(i);
            assert(slot.size() == slot_size);
            assert(slot[0] == static_cast<std::byte>(i + 1));
        }
    }
    std::cout << "- test_take passed" << std::endl;
}

int main() {
    test_new_constructor();
    test_insert();
    test_split();
    test_merge();
    test_take();
    std::cout << "All tests passed." << std::endl;
    return 0;
}