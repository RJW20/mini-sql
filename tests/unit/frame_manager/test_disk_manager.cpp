#include "frame_manager/disk_manager/disk_manager.hpp"

#include <cassert>
#include <iostream>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <vector>

#include "frame_manager/disk_manager/page_id_t.hpp"
#include "exceptions/engine_exceptions.hpp"
#include "byte_io.hpp"

#include "utils.hpp"

using namespace minisql;

void test_constructor() {
    std::filesystem::path path = make_temp_path();
    const std::streamoff base_offset = 32;
    const std::size_t page_size = 4096;
    const page_id_t page_count = 100;
    create_file(path, base_offset + page_size * page_count);
    std::fstream file{path, std::ios::binary | std::ios::in | std::ios::out};
    {
        DiskManager disk{file, base_offset, page_size, page_count};
        assert(disk.page_size() == page_size);
        assert(disk.page_count() == page_count);
    }
    try {
        DiskManager disk{file, base_offset - 1, page_size, page_count};
        assert(false);
    }
    catch (const DiskException&) {}
    try {
        DiskManager disk{file, base_offset + 1, page_size, page_count};
        assert(false);
    }
    catch (const DiskException&) {}
    delete_path(path);
    std::cout << "- test_constructor passed" << std::endl;
}

void test_extend() {
    std::filesystem::path path = make_temp_path();
    create_file(path);
    std::fstream file{path, std::ios::binary | std::ios::in | std::ios::out};
    {
        DiskManager disk{file, 0, 4096, 0};
        for (int i = 0; i < 100; i++) {
            assert(disk.page_count() == i);
            disk.extend();
        }
    }
    delete_path(path);
    std::cout << "- test_extend passed" << std::endl;
}

void test_write_read() {
    std::filesystem::path path = make_temp_path();
    const std::size_t page_size = 4096;
    create_file(path);
    std::fstream file{path, std::ios::binary | std::ios::in | std::ios::out};
    {
        DiskManager disk{file, 0, page_size, 0};
        disk.extend();
        std::vector<std::byte> src(page_size);
        std::size_t i = 0;
        while (true) {
            try {
                byte_io::write<double>(src, i, i);
                i = i + sizeof(double);
            }
            catch (const ByteIOException&) { break; }
        }
        disk.write(0, src.data());
        std::vector<std::byte> dst(page_size);
        disk.read(0, dst.data());
        for (int i = 0; i < page_size; i++) assert(src[i] == dst[i]);
        try {
            disk.write(1, src.data());
            assert(false);
        }
        catch (const DiskException&) {}
        try {
            disk.read(1, dst.data());
            assert(false);
        }
        catch (const DiskException&) {}
    }
    delete_path(path);
    std::cout << "- test_write_read passed" << std::endl;
}

int main() {
    test_constructor();
    test_extend();
    test_write_read();
    std::cout << "All tests passed." << std::endl;
    return 0;
}