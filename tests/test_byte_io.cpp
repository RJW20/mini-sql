#include "byte_io.hpp"

#include <cassert>
#include <iostream>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <typeinfo>

using namespace minisql;

template <typename T>
void test_write_copy(T t) {
    std::vector<std::byte> bytes(20);
    const std::size_t test_offset = 0;
    ByteIO::write<T>(bytes, test_offset, t);
    assert(ByteIO::copy<T>(bytes, test_offset) == t);
    std::cout << "- test_write_copy passed" << std::endl;
}

template <typename T>
void test_out_of_range(T t) {
    std::vector<std::byte> bytes(20);
    int test_offset = bytes.size() - sizeof(T) + 1;
    try {
        ByteIO::write<T>(bytes, test_offset, t);
        assert(false);
    }
    catch (const std::out_of_range&) {}
    try {
        ByteIO::copy<T>(bytes, test_offset);
        assert(false);
    }
    catch (const std::out_of_range&) {}
    std::cout << "- test_out_of_range passed" << std::endl;
}

template <typename T>
void run_tests(T t) {
    std::cout << "Running tests for " << typeid(T).name() << ":" << std::endl;
    test_write_copy<T>(t);
    test_out_of_range<T>(t);
}

int main() {
    run_tests<std::uint8_t>(0x80);
    run_tests<std::uint16_t>(0x8000);
    run_tests<int>(0x80000000);
    run_tests<double>(5555.5555);
    std::cout << "All tests passed." << std::endl;
    return 0;
}