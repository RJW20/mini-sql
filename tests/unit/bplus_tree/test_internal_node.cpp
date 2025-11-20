#include "bplus_tree/internal_node.hpp"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <typeinfo>

#include "bplus_tree/node.hpp"
#include "field/varchar.hpp"
#include "frame_manager/cache/frame.hpp"
#include "frame_manager/cache/frame_view.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"

#include "utils.hpp"

using namespace minisql;

template <typename Key>
void test_new_constructor() {
    Frame f;
    f.data.resize(2048);
    const page_id_t first_child = generate<page_id_t>();
    InternalNode node{
        FrameView{nullptr, &f}, key_size<Key>(), generate<page_id_t>(),
        first_child
    };
    assert(!node.is_leaf());
    assert(node.child(-1) == first_child);
    std::cout << "- test_new_constructor passed" << std::endl;
}

template <typename Key>
void test_insert() {
    Frame f;
    f.data.resize(2048);
    const Node::key_size_t key_size_ = key_size<Key>();
    const Node::size_t max_slots =
        (f.data.size() - InternalNodeHeader::SIZE) /
        (key_size_ + sizeof(page_id_t));
    InternalNode node{
        FrameView{nullptr, &f}, key_size_, generate<page_id_t>()
    };
    for (int i = 0; i < max_slots; i++) {
        const Key key = generate<Key>(i);
        const page_id_t child = generate<page_id_t>(i);
        node.insert<Key>(i, key, child);
        assert(node.size() == i + 1);
        assert(node.key<Key>(i) == key);
        assert(node.child(i) == child);
    }
    assert(node.at_max_capacity());
    std::cout << "- test_insert passed" << std::endl;
}

template <typename Key>
void test_split() {
    Frame f1, f2;
    const std::size_t page_size = 2048;
    f1.data.resize(page_size);
    f2.data.resize(page_size);
    const Node::key_size_t key_size_ = key_size<Key>();
    const Node::size_t max_slots =
        (page_size - InternalNodeHeader::SIZE) /
        (key_size_ + sizeof(page_id_t));

    InternalNode dst{
        FrameView{nullptr, &f1}, key_size_, generate<page_id_t>()
    };
    InternalNode src{
        FrameView{nullptr, &f2}, key_size_, generate<page_id_t>(),
        generate<page_id_t>(-1)
    };

    for (int i = 0; i < max_slots; i++)
        src.insert(i, generate<Key>(i), generate<page_id_t>(i));

    const Node::size_t middle_slot = max_slots / 2 + max_slots % 2 - 1;
    assert(InternalNode::split<Key>(&dst, &src) == generate<Key>(middle_slot));
    assert(dst.size() == max_slots - middle_slot - 1);
    assert(dst.child(-1) == generate<page_id_t>(middle_slot));
    for (int i = 0; i < dst.size(); i++) {
        assert(dst.key<Key>(i) == generate<Key>(i + middle_slot + 1));
        assert(dst.child(i) == generate<page_id_t>(i + middle_slot + 1));
    }
    assert(src.size() == middle_slot);
    assert(src.child(-1) == generate<page_id_t>(-1));
    for (int i = 0; i < src.size(); i++) {
        assert(src.key<Key>(i) == generate<Key>(i));
        assert(src.child(i) == generate<page_id_t>(i));
    }

    std::cout << "- test_split passed" << std::endl;
}

template <typename Key>
void test_merge() {
    Frame f1, f2;
    const std::size_t page_size = 2048;
    f1.data.resize(page_size);
    f2.data.resize(page_size);
    const Node::key_size_t key_size_ = key_size<Key>();
    const Node::size_t max_slots =
        (page_size - InternalNodeHeader::SIZE) /
        (key_size_ + sizeof(page_id_t));
    const Node::size_t min_slots = max_slots / 2 + max_slots % 2 - 1;

    InternalNode dst{
        FrameView{nullptr, &f1}, key_size_, generate<page_id_t>(),
        generate<page_id_t>(-1)
    };
    InternalNode src{
        FrameView{nullptr, &f2}, key_size_, generate<page_id_t>(),
        generate<page_id_t>(-1)
    };

    for (int i = 0; i < min_slots; i++) {
        dst.insert(i, generate<Key>(i), generate<page_id_t>(i));
        src.insert(i, generate<Key>(i), generate<page_id_t>(i));
    }

    const Key separator = generate<Key>(min_slots);
    InternalNode::merge<Key>(&dst, &src, separator);
    assert(dst.size() == 2 * min_slots + 1);
    assert(dst.child(-1) == generate<page_id_t>(-1));
    for (int i = 0; i < min_slots; i++) {
        assert(dst.key<Key>(i) == generate<Key>(i));
        assert(dst.child(i) == generate<page_id_t>(i));
    }
    assert(dst.key<Key>(min_slots) == separator);
    assert(dst.child(min_slots) == generate<page_id_t>(-1));
    for (int i = min_slots + 1; i < dst.size(); i++) {
        assert(dst.key<Key>(i) == generate<Key>(i - min_slots - 1));
        assert(dst.child(i) == generate<page_id_t>(i - min_slots - 1));
    }
    assert(!src.size());

    std::cout << "- test_merge passed" << std::endl;
}

template <typename Key>
void test_take() {
    Frame f1, f2;
    const std::size_t page_size = 2048;
    f1.data.resize(page_size);
    f2.data.resize(page_size);
    const Node::key_size_t key_size_ = key_size<Key>();
    const Node::size_t max_slots =
        (page_size - InternalNodeHeader::SIZE) /
        (key_size_ + sizeof(page_id_t));
    {
        InternalNode dst{
            FrameView{nullptr, &f1}, key_size_, generate<page_id_t>()
        };
        InternalNode src{
            FrameView{nullptr, &f2}, key_size_, generate<page_id_t>(),
            generate<page_id_t>(-1)
        };

        for (int i = 0; i < max_slots - 1; i++)
            dst.insert(i, generate<Key>(i), generate<page_id_t>(i));
        for (int i = 0; i < max_slots; i++)
            src.insert(i, generate<Key>(i), generate<page_id_t>(i));

        const Key separator = generate<Key>(max_slots - 1);
        assert(
            InternalNode::take_back<Key>(&dst, &src, separator) ==
            generate<Key>(max_slots - 1)
        );
        assert(dst.size() == max_slots);
        assert(dst.child(-1) == generate<page_id_t>(max_slots - 1));
        assert(dst.key<Key>(0) == separator);
        assert(dst.child(0) == generate<page_id_t>(-1));
        for (int i = 1; i < dst.size(); i++) {
            assert(dst.key<Key>(i) == generate<Key>(i - 1));
            assert(dst.child(i) == generate<page_id_t>(i - 1));
        }
        assert(src.size() == max_slots - 1);
        assert(src.child(-1) == generate<page_id_t>(-1));
        for (int i = 0; i < src.size(); i++) {
            assert(src.key<Key>(i) == generate<Key>(i));
            assert(src.child(i) == generate<page_id_t>(i));
        }
    }
    {
        InternalNode dst{
            FrameView{nullptr, &f1}, key_size_, generate<page_id_t>()
        };
        InternalNode src{
            FrameView{nullptr, &f2}, key_size_, generate<page_id_t>(),
            generate<page_id_t>(-1)
        };

        for (int i = 0; i < max_slots - 1; i++)
            dst.insert(i, generate<Key>(i), generate<page_id_t>(i));
        for (int i = 0; i < max_slots; i++)
            src.insert(i, generate<Key>(i), generate<page_id_t>(i));

        const Key separator = generate<Key>(-1);
        assert(
            InternalNode::take_front<Key>(&dst, &src, separator) ==
            generate<Key>(0)
        );
        assert(dst.size() == max_slots);
        assert(dst.child(-1) == generate<page_id_t>(-1));
        for (int i = 0; i < dst.size() - 1; i++) {
            assert(dst.key<Key>(i) == generate<Key>(i));
            assert(dst.child(i) == generate<page_id_t>(i));
        }
        assert(dst.key<Key>(dst.size() - 1) == separator);
        assert(dst.child(dst.size() - 1) == generate<page_id_t>(-1));
        assert(src.size() == max_slots - 1);
        assert(src.child(-1) == generate<page_id_t>(0));
        for (int i = 0; i < src.size(); i++) {
            assert(src.key<Key>(i) == generate<Key>(i + 1));
            assert(src.child(i) == generate<page_id_t>(i + 1));
        }
    }
    std::cout << "- test_take passed" << std::endl;
}

template <typename Key>
void run_tests() {
    std::cout << "Running tests for " << typeid(Key).name() << ":" << std::endl;
    test_new_constructor<Key>();
    test_insert<Key>();
    test_split<Key>();
    test_merge<Key>();
    test_take<Key>();
}

int main() {
    run_tests<int>();
    run_tests<double>();
    run_tests<Varchar>();
    std::cout << "All tests passed." << std::endl;
    return 0;
}