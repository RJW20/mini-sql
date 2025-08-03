#include "bplus_tree/node.hpp"
#include "bplus_tree/test_node.hpp"

#include <cassert>
#include <iostream>

#include "frame_manager/cache/frame.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "frame_manager/cache/frame_view.hpp"
#include "headers.hpp"
#include "exceptions.hpp"
#include "varchar.hpp"

#include "utils.hpp"

using namespace minisql;

template <typename Key>
void test_new_constructor() {
    Frame f;
    f.data.resize(2048);
    const page_id_t parent = generate<page_id_t>();
    TestNode node{FrameView{nullptr, &f}, key_size<Key>(), parent};
    assert(node.is_root() == (parent == nullpid));
    assert(node.pid() == f.pid);
    assert(node.parent() == parent);
    assert(!node.size());
    std::cout << "- test_new_constructor passed" << std::endl;
}

/* Tests:
 * - shifting a slot 1 to the right and setting that slot (basic insert)
 * - shifting a slot 1 to the left (basic erase)
 * - shifting a group to the right
 * - shifting a group to the left */
template <typename Key>
void test_shift() {
    Frame f;
    f.data.resize(2048);
    const Node::key_size_t key_size_ = key_size<Key>();
    const Node::size_t max_slots =
        (f.data.size() - NodeHeader::SIZE) / key_size_;
    {
        TestNode node{
            FrameView{nullptr, &f}, key_size_, generate<page_id_t>()
        };

        for (int i = 0; i < max_slots; i++) {
            node.insert<Key>(i, generate<Key>(i));
            assert(node.size() == i + 1);
            assert(node.key<Key>(i) == generate<Key>(i));
        }
        assert(node.at_max_capacity());

        for (int i = 0; i < max_slots; i++) {
            node.erase(0);
            assert(node.size() == max_slots - i - 1);
            if (node.size()) assert(node.key<Key>(0) == generate<Key>(i + 1));
        }

        const Node::size_t shift_start = max_slots / 2;
        const Node::size_t shift_steps = max_slots / 4;
        for (int i = 0; i < max_slots - shift_steps; i++)
            node.insert<Key>(i, generate<Key>(i));
        node.shift(shift_start, shift_steps);
        assert(node.size() == max_slots);
        for (int i = 0; i < shift_start; i++)
            assert(node.key<Key>(i) == generate<Key>(i));
        for (int i = shift_start + shift_steps; i < max_slots; i++)
            assert(node.key<Key>(i) == generate<Key>(i - shift_steps));

        node.shift(shift_start + shift_steps, - shift_steps);
        assert(node.size() == max_slots - shift_steps);
        for (int i = 0; i < max_slots - shift_steps; i++)
            assert(node.key<Key>(i) == generate<Key>(i));
    }
    std::cout << "- test_shift passed" << std::endl;
}

template <typename Key>
void test_assert_compatiblity() {
    Frame f1, f2;
    const std::size_t page_size = 2048;
    f1.data.resize(page_size);
    f2.data.resize(page_size);
    const Node::key_size_t key_size_ = key_size<Key>();
    TestNode n1{FrameView{nullptr, &f1}, key_size_, generate<page_id_t>()};
    {
        TestNode n2{FrameView{nullptr, &f2}, key_size_, generate<page_id_t>()};
        TestNode::assert_compatibility(&n1, &n2);
    }
    {
        TestNode n2(
            FrameView{nullptr, &f2}, key_size_ + 1, generate<page_id_t>()
        );
        try{
            TestNode::assert_compatibility(&n1, &n2);
            assert(false);
        }
        catch (const NodeIncompatibilityException&) {}
    }
    std::cout << "- test_assert_compatibility passed" << std::endl;
}

template <typename Key>
void test_splice() {
    Frame f1, f2;
    const std::size_t page_size = 2048;
    f1.data.resize(page_size);
    f2.data.resize(page_size);
    const Node::key_size_t key_size_ = key_size<Key>();
    const Node::size_t max_slots = (page_size - NodeHeader::SIZE) / key_size_;
    const Node::size_t splice_count = max_slots / 4;
    {
        TestNode dst{
            FrameView{nullptr, &f1}, key_size_, generate<page_id_t>()
        };
        TestNode src{
            FrameView{nullptr, &f2}, key_size_, generate<page_id_t>()
        };

        for (int i = 0; i < max_slots - splice_count; i++)
            dst.insert(i, generate<Key>(i));
        for (int i = 0; i < max_slots; i++) src.insert(i, generate<Key>(i));

        TestNode::splice_back_to_front(&dst, &src, splice_count);
        assert(dst.size() == max_slots);
        for (int i = 0; i < splice_count; i++)
            assert(
                dst.key<Key>(i) ==
                generate<Key>(i + max_slots - splice_count)
            );
        for (int i = splice_count; i < max_slots; i++)
            assert(dst.key<Key>(i) == generate<Key>(i - splice_count));
        assert(src.size() == max_slots - splice_count);
        for (int i = 0; i < max_slots - splice_count; i++)
            assert(src.key<Key>(i) == generate<Key>(i));
    }
    {
        TestNode dst{
            FrameView{nullptr, &f1}, key_size_, generate<page_id_t>()
        };
        TestNode src{
            FrameView{nullptr, &f2}, key_size_, generate<page_id_t>()
        };

        for (int i = 0; i < max_slots - splice_count; i++)
            dst.insert(i, generate<Key>(i));
        for (int i = 0; i < max_slots; i++) src.insert(i, generate<Key>(i));

        TestNode::splice_front_to_back(&dst, &src, splice_count);
        assert(dst.size() == max_slots);
        for (int i = 0; i < max_slots - splice_count; i++)
            assert(dst.key<Key>(i) == generate<Key>(i));
        for (int i = max_slots - splice_count; i < max_slots; i++)
            assert(
                dst.key<Key>(i) ==
                generate<Key>(i - (max_slots - splice_count))
            );
        assert(src.size() == max_slots - splice_count);
        for (int i = 0; i < max_slots - splice_count; i++)
            assert(src.key<Key>(i) == generate<Key>(i + splice_count));
    }
    std::cout << "- test_splice passed" << std::endl;
}

template <typename Key>
void test_read_constructor() {
    Frame f;
    f.data.resize(2048);
    const page_id_t parent = generate<page_id_t>();
    const Node::key_size_t key_size_ = key_size<Key>();
    const Node::size_t max_slots =
        (f.data.size() - NodeHeader::SIZE) / key_size_;
    TestNode node{FrameView{nullptr, &f}, key_size_, parent};
    for (int i = 0; i < max_slots; i++) node.insert<Key>(i, generate<Key>(i));
    TestNode loaded_node{FrameView{nullptr, &f}};
    assert(node.is_root() == (parent == nullpid));
    assert(node.pid() == f.pid);
    assert(node.parent() == parent);
    assert(node.size() == max_slots);
    for (int i = 0; i < max_slots; i++)
        assert(node.key<Key>(i) == generate<Key>(i));
    std::cout << "- test_read_constructor passed" << std::endl;
}

template <typename Key>
void run_tests() {
    std::cout << "Running tests for " << typeid(Key).name() << ":" << std::endl;
    test_new_constructor<Key>();
    test_shift<Key>();
    test_assert_compatiblity<Key>();
    test_splice<Key>();
    test_read_constructor<Key>();
}

int main() {
    run_tests<int>();
    run_tests<double>();
    run_tests<Varchar>();
    std::cout << "All tests passed." << std::endl;
    return 0;
}