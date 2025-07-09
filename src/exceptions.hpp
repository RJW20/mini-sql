#ifndef MINISQL_EXCEPTIONS_HPP
#define MINISQL_EXCEPTIONS_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "headers.hpp"

namespace minisql {

// DiskException for files that are not the correct size.
class DiskException : public std::runtime_error {
public:
    DiskException(std::size_t expected, std::size_t actual)
        : std::runtime_error(
            "Incorrect file size: expected " + std::to_string(expected) +
            " bytes, got " + std::to_string(actual) + " bytes."
        ) {}
};

// InvalidMagicException for headers that have an invalid magic.
class InvalidMagicException : public std::runtime_error {
public:
    InvalidMagicException(Magic magic) : std::runtime_error(
        "Invalid header format: bad magic number " +
        std::to_string(static_cast<std::underlying_type_t<Magic>>(magic)) + "."
    ) {}
};

} // namespace minisql

#endif // MINISQL_EXCEPTIONS_HPP