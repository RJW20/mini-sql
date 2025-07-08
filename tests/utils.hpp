#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <string>

#include "varchar.hpp"

template <typename T>
struct default_size {
    static_assert(std::is_arithmetic_v<T>);
    static constexpr std::size_t value = sizeof(T);
};

inline constexpr std::size_t TEST_VARCHAR_SIZE = 10;
template <>
struct default_size<minisql::Varchar> {
    static constexpr std::size_t value = TEST_VARCHAR_SIZE; 
};

template <typename T>
T generate_new(std::size_t size = default_size<T>::value) {
    static_assert(std::is_arithmetic_v<T>);
    static int seed = 0;
    return seed++ + 0.5;
}

template <>
inline minisql::Varchar generate_new<minisql::Varchar>(std::size_t size) {
    static int seed = 0;
    std::string underlying = "v" + std::to_string(seed++);
    return minisql::Varchar(underlying.c_str(), size);
}

#endif // UTILITIES_HPP