#include "byte_io.hpp"

#include <cassert>
#include <iostream>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <type_traits>

#include "utils.hpp"

using namespace minisql;

template <typename T>
void test_write_read() {
    std::vector<std::byte> bytes(20);
    const std::size_t test_offset = 0;
    T test_case = generate_new<T>();
    ByteIO::write<T>(bytes, test_offset, test_case);
    assert(ByteIO::read<T>(bytes, test_offset) == test_case);
    std::cout << "- test_write_read passed" << std::endl;
}

template <typename T>
void run_tests() {
    std::cout << "Running tests for " << typeid(T).name() << ":" << std::endl;
    test_write_read<T>();
}

int main() {
    run_tests<std::uint8_t>();
    run_tests<std::uint16_t>();
    run_tests<int>();
    run_tests<double>();
    std::cout << "All tests passed." << std::endl;
    return 0;
}