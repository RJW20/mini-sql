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

// NodeIncompatibilityException for Nodes with different key or slot sizes.
class NodeIncompatibilityException : public std::runtime_error {
public:
    using key_size_t = NodeHeader::key_size_t;
    using slot_size_t = NodeHeader::slot_size_t;

    NodeIncompatibilityException(
        key_size_t key_size_1, key_size_t key_size_2,
        slot_size_t slot_size_1, slot_size_t slot_size_2
    ) : std::runtime_error(
            "Incompatible nodes: key_sizes = " + std::to_string(key_size_1) +
            "," + std::to_string(key_size_2) + ", slot_sizes = " +
            std::to_string(slot_size_1) + "," + std::to_string(slot_size_2) +
            "."
    ) {}
};

// DBConstraintViolation for failed DDL/DML instructions.
class DBConstraintViolation : public std::runtime_error {
public:
    DBConstraintViolation(const std::string& arg) : std::runtime_error(arg) {}
};

} // namespace minisql

#endif // MINISQL_EXCEPTIONS_HPP