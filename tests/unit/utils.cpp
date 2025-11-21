#include "utils.hpp"

#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <minisql/varchar.hpp>

#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"

// ----------------------------------------------------------------------------
// Temporary file management
// ----------------------------------------------------------------------------

std::filesystem::path make_temp_path() {
    char path[L_tmpnam];
    std::tmpnam(path);
    return std::filesystem::path(path);
}

void create_file(const std::filesystem::path& path, std::size_t size) {
    std::fstream file{path, std::ios::out | std::ios::binary};
    if (size) {
        std::vector<char> zeros(size);
        file.write(zeros.data(), zeros.size());
    }
}

void delete_path(const std::filesystem::path& path) {
    std::remove(path.string().c_str());
}

// ----------------------------------------------------------------------------
// Node page_id_t/key generation
// ----------------------------------------------------------------------------
using namespace minisql;

template <>
page_id_t generate<page_id_t>(
    int seed, const std::string& prefix, std::size_t size
) { return seed; }

template <>
int generate<int>(int seed, const std::string& prefix, std::size_t size) {
    return seed;
}

template <>
double generate<double>(int seed, const std::string& prefix, std::size_t size)
{
    return seed + 0.5;
}

template <>
Varchar generate<Varchar>(
    int seed, const std::string& prefix, std::size_t size
) {
    std::string underlying = prefix + "_" + std::to_string(seed);
    return Varchar{underlying.c_str(), size};
}

template <>
NodeHeader::key_size_t key_size<int>() { return sizeof(int); }

template <>
NodeHeader::key_size_t key_size<double>() { return sizeof(double); }

template <>
NodeHeader::key_size_t key_size<Varchar>() { return DEFAULT_VARCHAR_SIZE; }