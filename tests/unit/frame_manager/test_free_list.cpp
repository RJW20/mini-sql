#include "frame_manager/free_list/free_list.hpp"

#include <cassert>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <vector>

#include "frame_manager/cache/cache.hpp"
#include "frame_manager/disk_manager/disk_manager.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"

#include "utils.hpp"

using namespace minisql;

void test_constructor() {
    std::filesystem::path path = make_temp_path();
    create_file(path);
    std::fstream file{path, std::ios::binary | std::ios::in | std::ios::out};
    {
        DiskManager disk{file, 0, 2048, 0};
        Cache cache{disk, 2000};
        FreeList free_list{cache, nullpid};
        assert(free_list.empty());
    }
    delete_path(path);
    std::cout << "- test_constructor passed" << std::endl;
}

void test_push_pop() {
    std::filesystem::path path = make_temp_path();
    create_file(path);
    std::fstream file{path, std::ios::binary | std::ios::in | std::ios::out};
    {
        DiskManager disk{file, 0, 2048, 0};
        Cache cache{disk, 2000};
        FreeList free_list{cache, nullpid};
        
        const std::size_t pids_per_block =
            (disk.page_size() - FreeListBlockHeader::SIZE) / sizeof(page_id_t);
        const std::size_t free_list_size = pids_per_block * 10;
        std::vector<page_id_t> pids(free_list_size);
        for (page_id_t pid = 0; pid < pids.size(); pid++) {
            disk.extend();
            pids[pid] = pid;
        }

        for (page_id_t pid : pids) free_list.push_back(pid);

        while (pids.size()) {
            assert(free_list.pop_back() == pids.back());
            pids.pop_back();
        }
        assert(free_list.empty());
    }
    delete_path(path);
    std::cout << "- test_push_pop passed" << std::endl;
}

int main() {
    test_constructor();
    test_push_pop();
    std::cout << "All tests passed." << std::endl;
    return 0;
}