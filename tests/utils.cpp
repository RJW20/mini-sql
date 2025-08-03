#include "utils.hpp"

#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <utility>

#include "varchar.hpp"
#include "headers.hpp"
#include "row/field.hpp"
#include "row/schema.hpp"
#include "row/row.hpp"

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