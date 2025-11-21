#include "byte_io.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <typeinfo>
#include <vector>

#include <minisql/varchar.hpp>

#include "exceptions/engine_exceptions.hpp"

using namespace minisql;

template <typename T>
void test_write_copy(T t, std::size_t size = sizeof(T)) {
    std::vector<std::byte> bytes(20);
    const std::size_t test_offset = 0;
    byte_io::write<T>(bytes, test_offset, t);
    assert(byte_io::copy<T>(bytes, test_offset, size) == t);
    std::cout << "- test_write_copy passed" << std::endl;
}

template <typename T>
void test_out_of_range(T t, std::size_t size = sizeof(T)) {
    std::vector<std::byte> bytes(20);
    int test_offset = bytes.size() - size + 1;
    try {
        byte_io::write<T>(bytes, test_offset, t);
        assert(false);
    }
    catch (const ByteIOException&) {}
    try {
        byte_io::copy<T>(bytes, test_offset, size);
        assert(false);
    }
    catch (const ByteIOException&) {}
    std::cout << "- test_out_of_range passed" << std::endl;
}

template <typename T>
void run_tests(T t, std::size_t size = sizeof(T)) {
    std::cout << "Running tests for " << typeid(T).name() << ":" << std::endl;
    test_write_copy<T>(t, size);
    test_out_of_range<T>(t, size);
}

int main() {
    run_tests<std::uint8_t>(0x80);
    run_tests<std::uint16_t>(0x8000);
    run_tests<int>(0x80000000);
    run_tests<double>(5555.5555);
    run_tests<Varchar>({"test", 5}, 5);
    std::cout << "All tests passed." << std::endl;
    return 0;
}