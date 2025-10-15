#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include <filesystem>

#include "headers.hpp"

// Temporary file management
std::filesystem::path make_temp_path();
void create_file(const std::filesystem::path& path, std::size_t size = 0);
void delete_path(const std::filesystem::path& path);

// Node page_id_t/key generation
static int global_seed = 0;
inline constexpr std::size_t DEFAULT_VARCHAR_SIZE = 10;
template <typename T>
T generate(
    int seed = global_seed, const std::string& prefix = "v",
    std::size_t = DEFAULT_VARCHAR_SIZE
);
template <typename Key>
minisql::NodeHeader::key_size_t key_size();

#endif // UTILS_HPP