#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <cstddef>
#include <cstdio>
#include <type_traits>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>

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

inline std::filesystem::path make_temp_path() {
    char path[L_tmpnam];
    std::tmpnam(path);
    return std::filesystem::path(path);
}

inline void create_file(
    const std::filesystem::path& path, std::size_t size = 0
) {
    std::fstream file{path, std::ios::out | std::ios::binary};
    if (size) {
        std::vector<char> zeros(size);
        file.write(zeros.data(), zeros.size());
    }
}

inline void delete_path(const std::filesystem::path& path) {
    std::remove(path.string().c_str());
}

#endif // UTILITIES_HPP