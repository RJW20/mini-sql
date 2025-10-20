#include "field/varchar.hpp"

#include <cassert>
#include <iostream>
#include <cstddef>
#include <utility>
#include <vector>
#include <functional>

#include "byte_io.hpp"

using namespace minisql;

void test_constructor() {
    const char* input = "test";
    Varchar v(input, 4);
    assert(v.size() == 4);
    for (int i = 0; i < 4; ++i) {
        assert(v.data()[i] == input[i]);
    }
    assert(v.data()[4] == '\0');
    std::cout << "- test_constructor passed" << std::endl;
}

void test_view_constructor() {
    char input[8] = "test";
    Varchar v(input, 8);
    assert(v.size() == 8);
    for (int i = 0; i < 8; ++i) {
        assert(v.data()[i] == input[i]);
    }
    input[4] = 't';
    input[5] = 'e';
    input[6] = 's';
    input[7] = 't';
        for (int i = 0; i < 8; ++i) {
        assert(v.data()[i] == input[i]);
    }
    std::cout << "- test_view_constructor passed" << std::endl;
}

void test_equality_operator() {
    const char* input = "testing";
    const int size = 10;
    assert(Varchar(input, size) == Varchar(input, size));
    std::cout << "- test_equality_operator passed" << std::endl;
}

void test_copy_constructor() {
    Varchar v1("testing", 10);
    Varchar v2 = v1;
    assert(v1 == v2);
    Varchar v3(v1.data(), v1.size());
    Varchar v4 = v3;
    assert(v3 == v4);
    std::cout << "- test_copy_constructor passed" << std::endl;
}

void test_copy_assignment() {
    Varchar v1("testing", 10);
    Varchar v2("decoy", 5);
    v2 = v1;
    Varchar v3(v1.data(), v1.size());
    Varchar v4(v2.data(), v2.size());
    v4 = v3;
    assert(v3 == v4);
    std::cout << "- test_copy_assignment passed" << std::endl;
}

void test_move_constructor() {
    Varchar v1("testing", 10);
    Varchar v2 = std::move(v1);
    assert(v1 == v2);
    Varchar v3(v1.data(), v1.size());
    Varchar v4 = std::move(v3);
    assert(v3 == v4);
    std::cout << "- test_move_constructor passed" << std::endl;
}

void test_move_assignment() {
    Varchar v1("testing", 10);
    Varchar v2("decoy", 5);
    v2 = std::move(v1);
    Varchar v3(v1.data(), v1.size());
    Varchar v4(v2.data(), v2.size());
    v4 = std::move(v3);
    assert(v3 == v4);
    std::cout << "- test_move_assignment passed" << std::endl;
}

void test_own_data() {
    Varchar v1("testing", 10);
    Varchar v2 = v1;
    {
        Varchar v3 = std::move(v1);
        v1.own_data();
    }
    assert(v1 == v2);
    std::cout << "- test_own_data passed" << std::endl;
}

void test_less_than_operator() {
    assert(Varchar("a", 1) < Varchar("b", 1));
    assert(Varchar("a", 2) < Varchar("b", 1));
    assert(Varchar("a", 1) < Varchar("a", 2));
    std::cout << "- test_less_than_operator passed" << std::endl;
}

void test_byte_io_write_copy_view() {
    std::vector<std::byte> bytes(20);
    const std::size_t test_offset = 0;
    Varchar v("testing", 10);
    byte_io::write<Varchar>(bytes, test_offset, v);
    assert(byte_io::copy<Varchar>(bytes, test_offset, v.size()) == v);
    assert(byte_io::view<Varchar>(bytes, test_offset, v.size()) == v);
    std::cout << "- test_byte_io_write_copy_view passed" << std::endl;
}

int main() {
    test_constructor();
    test_view_constructor();
    test_equality_operator();
    test_copy_constructor();
    test_copy_assignment();
    test_move_constructor();
    test_move_assignment();
    test_own_data();
    test_less_than_operator();
    test_byte_io_write_copy_view();
    std::cout << "All tests passed." << std::endl;
    return 0;
}