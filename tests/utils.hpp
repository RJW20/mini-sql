#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include <filesystem>
#include <memory>

#include "varchar.hpp"
#include "headers.hpp"
#include "row/field.hpp"
#include "row/schema.hpp"
#include "row/row.hpp"

// Temporary file management
std::filesystem::path make_temp_path();
void create_file(const std::filesystem::path& path, std::size_t size = 0);
void delete_path(const std::filesystem::path& path);

#endif // UTILS_HPP