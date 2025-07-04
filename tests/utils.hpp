#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <string>

#include "varchar.hpp"

inline constexpr std::size_t TEST_VARCHAR_SIZE = 10;

template <typename T>
T generate_new(std::size_t length = TEST_VARCHAR_SIZE) {
    static int seed = 0;
    if constexpr (std::is_same_v<T, minisql::Varchar>) {
        std::string underlying = "v" + std::to_string(seed++);
        return minisql::Varchar(underlying.c_str(), length);
    }
    else {
        return seed++ + 0.5;
    }
}

#endif // UTILITIES_HPP