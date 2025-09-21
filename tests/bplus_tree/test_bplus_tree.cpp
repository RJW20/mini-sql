#include "bplus_tree/bplus_tree.hpp"

#include <cassert>
#include <iostream>
#include <cstddef>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <typeinfo>

#include "frame_manager/cache/frame.hpp"
#include "bplus_tree/node.hpp"
#include "headers.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "frame_manager/frame_manager.hpp"
#include "byte_io.hpp"
#include "varchar.hpp"

#include "utils.hpp"
#include "bplus_tree/test_node.hpp"

using namespace minisql;

template <typename Key>
void test_seek_slot() {
    Frame f;
    f.data.resize(2048);
    const Node::key_size_t key_size_ = key_size<Key>();
    const Node::size_t max_slots =
        (f.data.size() - NodeHeader::SIZE) / key_size_;

    TestNode node{
        FrameView{nullptr, &f}, key_size_, generate<page_id_t>()
    };

    std::vector<Key> keys;
    for (int i = 0; i < max_slots * 3; i++) keys.push_back(generate<Key>(i));
    std::sort(keys.begin(), keys.end());

    for (int i = 0; i < max_slots; i++) node.insert<Key>(i, keys[i * 3]);

    for (int i = 0; i < max_slots * 3; i++) {
        if (!(i % 3))
            assert(BPlusTree::seek_slot<Key>(&node, keys[i]) == i / 3);
        else assert(BPlusTree::seek_slot<Key>(&node, keys[i]) == i / 3 + 1);
    }

    std::cout << "- test_seek_slot passed" << std::endl;
}

template <typename Key>
void test_insert() {
    std::filesystem::path path = make_temp_path();
    create_file(path);
    std::fstream file{path, std::ios::binary | std::ios::in | std::ios::out};
    {
        const std::size_t page_size = 512;
        FrameManager fm{file, 0, page_size, 0, 1000};
        const Node::key_size_t key_size_ = key_size<Key>();
        const Node::size_t internal_max_slots =
            (page_size - InternalNodeHeader::SIZE) /
            (key_size_ + sizeof(page_id_t));
        const Node::size_t leaf_max_slots =
            (page_size - LeafNodeHeader::SIZE) / key_size_;
        const int depth = 4;
        assert(depth > 2);  // Ensures full testing
        const std::size_t max_slots =
            (internal_max_slots + 1) *
            std::pow(internal_max_slots, depth - 3) * leaf_max_slots;

        BPlusTree bp_tree{&fm, key_size_, key_size_};
        
        for (int i = 0; i < max_slots; i++) {
            const Key key = generate<Key>(i);
            auto leaf_node = bp_tree.seek_leaf<Key>(key);
            Node::size_t slot =
                BPlusTree::seek_slot<Key>(leaf_node.get(), key);
            std::vector<std::byte> bytes{key_size_};
            byte_io::write<Key>(bytes, 0, key);
            bp_tree.insert_into<Key>(leaf_node.get(), slot, bytes);
        }

        for (int i = 0; i < max_slots; i++) {
            const Key key = generate<Key>(i);
            auto leaf_node = bp_tree.seek_leaf<Key>(key);
            Node::size_t slot =
                BPlusTree::seek_slot<Key>(leaf_node.get(), key);
            assert(leaf_node->template key<Key>(slot) == key);
        }
    }
    delete_path(path);
    std::cout << "- test_insert passed" << std::endl;
}

template <typename Key>
void test_erase() {
    std::filesystem::path path = make_temp_path();
    create_file(path);
    std::fstream file{path, std::ios::binary | std::ios::in | std::ios::out};
    {
        const std::size_t page_size = 512;
        FrameManager fm{file, 0, page_size, 0, 1000};
        const Node::key_size_t key_size_ = key_size<Key>();
        const Node::size_t internal_max_slots =
            (page_size - InternalNodeHeader::SIZE) /
            (key_size_ + sizeof(page_id_t));
        const Node::size_t leaf_max_slots =
            (page_size - LeafNodeHeader::SIZE) / key_size_;
        const int depth = 4;
        assert(depth > 2);  // Ensures full testing
        const std::size_t max_slots =
            (internal_max_slots + 1) *
            std::pow(internal_max_slots, depth - 3) * leaf_max_slots;

        BPlusTree bp_tree{&fm, key_size_, key_size_};

        for (int i = 0; i < max_slots; i++) {
            const Key key = generate<Key>(i);
            auto leaf_node = bp_tree.seek_leaf<Key>(key);
            Node::size_t slot =
                BPlusTree::seek_slot<Key>(leaf_node.get(), key);
            std::vector<std::byte> bytes{key_size_};
            byte_io::write<Key>(bytes, 0, key);
            bp_tree.insert_into<Key>(leaf_node.get(), slot, bytes);
        }

        for (int i = 0; i < max_slots; i++) {
            const Key key = generate<Key>(i);
            auto leaf_node = bp_tree.seek_leaf<Key>(key);
            Node::size_t slot =
                BPlusTree::seek_slot<Key>(leaf_node.get(), key);
            assert(leaf_node->template key<Key>(slot) == key);
            bp_tree.erase_from<Key>(leaf_node.get(), slot);
            leaf_node = bp_tree.seek_leaf<Key>(key);
            slot = BPlusTree::seek_slot<Key>(leaf_node.get(), key);
            if (slot != leaf_node->size())
                assert(leaf_node->template key<Key>(slot) != key);
        }
    }
    delete_path(path);
    std::cout << "- test_erase passed" << std::endl;
}

template <typename Key>
void run_tests() {
    std::cout << "Running tests for " << typeid(Key).name() << ":" << std::endl;
    test_seek_slot<Key>();
    test_insert<Key>();
    test_erase<Key>();
}

int main() {
    run_tests<int>();
    run_tests<double>();
    run_tests<Varchar>();
    std::cout << "All tests passed." << std::endl;
    return 0;
}