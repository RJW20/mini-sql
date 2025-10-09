#include "frame_manager/cache/cache.hpp"

#include <cassert>
#include <iostream>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <vector>

#include "frame_manager/disk_manager/disk_manager.hpp"
#include "frame_manager/cache/frame_view.hpp"
#include "exceptions/engine_exceptions.hpp"

#include "utils.hpp"

using namespace minisql;

void test_constructor() {
    std::filesystem::path path = make_temp_path();
    create_file(path);
    std::fstream file{path, std::ios::binary | std::ios::in | std::ios::out};
    const std::size_t capacity = 2000;
    {
        DiskManager disk{file, 0, 2048, 0};
        Cache cache{disk, capacity};
        assert(cache.capacity() == capacity);
    }
    delete_path(path);
    std::cout << "- test_constructor passed" << std::endl;
}

/* Tests:
 * - pinning a pid not in map_
 * - pinning a pid in map_ with zero pin_count
 * - pinning a pid in map_ with non-zero pin_count */
void test_pin() {
    std::filesystem::path path = make_temp_path();
    create_file(path);
    std::fstream file{path, std::ios::binary | std::ios::in | std::ios::out};
    const std::size_t capacity = 2000;
    {
        DiskManager disk{file, 0, 2048, 0};
        for (int i = 0; i < capacity * 2; i++) disk.extend();
        Cache cache{disk, capacity};

        std::vector<FrameView> fvs;
        for (page_id_t pid = 0; pid < cache.capacity(); pid++) {
            fvs.push_back(cache.pin(pid));
            assert(fvs.back().pid() == pid);
        }
        try { 
            cache.pin(cache.capacity());
            assert(false);
        }
        catch (const CacheCapacityException&) {}
        fvs.clear();
        for (page_id_t pid = cache.capacity(); pid < disk.page_count(); pid++)
            assert(cache.pin(pid).pid() == pid);

        for (page_id_t pid = 0; pid < cache.capacity(); pid++) {
            cache.pin(pid);
            fvs.push_back(cache.pin(pid));
            assert(fvs.back().pid() == pid);
        }

        for (page_id_t pid = 0; pid < cache.capacity(); pid++)
            assert(cache.pin(pid).pid() == pid);
    }
    delete_path(path);
    std::cout << "- test_pin passed" << std::endl;
}

void test_unpin() {
    std::filesystem::path path = make_temp_path();
    create_file(path);
    std::fstream file{path, std::ios::binary | std::ios::in | std::ios::out};
    const std::size_t capacity = 2000;
    {
        DiskManager disk{file, 0, 2048, 0};
        for (int i = 0; i < capacity * 2; i++) disk.extend();
        Cache cache{disk, capacity};

        try {
            cache.unpin(0, false);
            assert(false);
        }
        catch (const CacheUnpinException&) {}

        try {
            cache.pin(0);
            cache.unpin(0, false);
            assert(false);
        }
        catch (const CacheUnpinException&) {}

        std::vector<FrameView> fvs;
        for (page_id_t pid = 0; pid < cache.capacity(); pid++)
            fvs.push_back(cache.pin(pid));
        fvs.clear();
    }
    delete_path(path);
    std::cout << "- test_unpin passed" << std::endl;
}

int main() {
    test_constructor();
    test_pin();
    test_unpin();
    std::cout << "All tests passed." << std::endl;
    return 0;
}