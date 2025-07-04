
#include <cassert>
#include <iostream>
#include <cstddef>
#include <stdexcept>
#include <cstring>

#include "varchar.hpp"

using namespace minisql;

void test_constructor() {
    const char* input = "test";
    Varchar v(input, 4);
    assert(v.size() == 4);
    for (int i = 0; i < 4; ++i) {
        assert(v.data()[i] == input[i]);
    }
    assert(v.data()[4] == '\0');
    std::cout << "const char* and size_t constructor test passed."
        << std::endl;
}

void test_size_t_constructor() {
    Varchar v(std::size_t(10));
    assert(v.size() == 10);
    for (std::size_t i = 0; i < v.size(); ++i) {
        assert(v.data()[i] == '\0');
    }
    std::cout << "size_t constructor test passed." << std::endl;
}

void test_char_constructor() {
    const char input = '(';
    Varchar v = input;
    assert(v.size() == 1);
    assert(v.data()[0] == input);
    std::cout << "const char constructor test passed." << std::endl;
}

void test_oversized_input() {
    try {
        Varchar v(std::size_t(300));
        assert(false);
    } catch (const std::length_error&) {
        std::cout << "Oversized input test passed." << std::endl;
    }
}

void test_equality_operator() {
    const char* input = "testing";
    const int size = 10;
    assert(Varchar(input, size) == Varchar(input, size));
    std::cout << "Equality test passed." << std::endl;
}

void test_copy_constructor() {
    Varchar v1("testing", 10);
    Varchar v2 = v1;
    assert(v1 == v2);
    std::cout << "Copy constructor test passed." << std::endl;
}

void test_copy_assignment() {
    Varchar v1("testing", 10);
    Varchar v2("decoy", 5);
    v2 = v1;
    assert(v1 == v2);
    std::cout << "Copy assignment test passed." << std::endl;
}

void test_less_than_operator() {
    assert(Varchar("a", 1) < Varchar("b", 1));
    assert(Varchar("a", 2) < Varchar("b", 1));
    assert(Varchar("a", 1) < Varchar("a", 2));
    std::cout << "Less than comparison test passed." << std::endl;
}

void test_conversion_operator() {
    const char* input = "test";
    Varchar v(input, 4);
    const char* cstr = v;
    assert(std::strcmp(cstr, "test") == 0);
    std::cout << "Conversion operator test passed." << std::endl;
}

int main() {
    test_constructor();
    test_size_t_constructor();
    test_char_constructor();
    test_oversized_input();
    test_equality_operator();
    test_copy_constructor();
    test_copy_assignment();
    test_less_than_operator();
    test_conversion_operator();
    std::cout << "All tests passed." << std::endl;
    return 0;
}