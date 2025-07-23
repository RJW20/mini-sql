#include "frame_manager/free_list/free_list_block.hpp"

#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstddef>
#include <vector>

#include "frame_manager/cache/frame.hpp"
#include "byte_io.hpp"
#include "headers.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "exceptions.hpp"

using namespace minisql;

void test_constructor() {
    Frame f;
    f.data.resize(2048);
    {
        FreeListBlock block{FrameView{nullptr, &f}, true};
        assert(
            ByteIO::view<Magic>(f.data, FreeListBlockHeader::MAGIC_OFFSET) ==
            Magic::FREE_LIST_BLOCK
        );
        assert(block.next_block() == nullpid);
        assert(block.empty());
    }
    ByteIO::write<std::uint8_t>(f.data, FreeListBlockHeader::MAGIC_OFFSET, -1);
    try {
        FreeListBlock block{FrameView{nullptr, &f}};
        assert(false);
    }
    catch (const InvalidMagicException&) {}
    std::cout << "- test_constructor passed" << std::endl;
}

void test_push_pop() {
    Frame f;
    f.data.resize(2048);
    FreeListBlock block{FrameView{nullptr, &f}, true};

    const std::size_t max_size = (f.data.size() - FreeListBlockHeader::SIZE) /
        sizeof(page_id_t);
    std::vector<page_id_t> pids(max_size);
    for (page_id_t pid = 0; pid < pids.size(); pid++) pids[pid] = pid;

    for (page_id_t pid : pids) block.push_back(pid);
    assert(block.full());

    while (pids.size()) {
        assert(block.pop_back() == pids.back());
        pids.pop_back();
    }
    assert(block.empty());
    std::cout << "- test_push_pop passed" << std::endl;
}

int main() {
    test_constructor();
    test_push_pop();
    std::cout << "All tests passed." << std::endl;
    return 0;
}