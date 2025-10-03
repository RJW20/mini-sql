#ifndef MINISQL_EXCEPTIONS_HPP
#define MINISQL_EXCEPTIONS_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "headers.hpp"
#include "field/field.hpp"

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

// Base class for an invalid Query.
class InvalidQueryException : public std::runtime_error {
public:
    InvalidQueryException(const std::string& s) : std::runtime_error(s) {}
    virtual ~InvalidQueryException() = default;
};

/* TableNameException for table names that already exist and cannot be used
 * again or that cannot be found. */
class TableNameException : public InvalidQueryException {
public:
    TableNameException(const std::string& name, bool exists)
        : InvalidQueryException(
            "Invalid table name: " + name +
            (exists ? " already exists." : " does not exist.")
        ) {}
};

/* ColumnNameException for the following scenarios:
 * - Column names that don't exist in a table.
 * - Disallowed column names.
 * - Incomplete list of column names.
 * - Attempted modifications to a constant column. */
class ColumnNameException : public InvalidQueryException {
public:
    enum class Reason { EXISTENCE, DISALLOWED, INCOMPLETE, CONSTANT };

    ColumnNameException(const std::string& name, Reason reason)
        : InvalidQueryException(generate_message(name, reason)) {}

private:
    static std::string generate_message(const std::string& name, Reason reason)
    {
        switch (reason) {
            case Reason::EXISTENCE:
                return "Invalid column name: " + name + " does not exist.";
            case Reason::DISALLOWED:
                return "Invalid column name: " + name + " is reserved.";
            case Reason::INCOMPLETE:
                return "Incomplete column list.";
            case Reason::CONSTANT:
                return "Column " + name + " cannot be modified.";
        }
        __builtin_unreachable();
    }
};

// FieldTypeException for valid Field types in invalid locations.
class FieldTypeException : public InvalidQueryException {
public:
    FieldTypeException(const std::string& expected, const std::string& actual)
        : InvalidQueryException(
            "Invalid Field type: expected " + expected + ", got " + actual +
            "."
        ) {}

    FieldTypeException(FieldType expected, FieldType actual)
        : InvalidQueryException(
            "Invalid Field type: expected " + to_string(expected) + ", got" +
            to_string(actual) + "."
        ) {}

private:
    static std::string to_string(FieldType type) {
        switch (type) {
            case FieldType::INT: return "INT";
            case FieldType::REAL: return "REAL";
            case FieldType::TEXT: return "TEXT";
        }
        __builtin_unreachable();
    }
};

// UnrecognisedSQLException for unsupported or invalid SQL tokens.
class UnrecognisedSQLException : public std::runtime_error {
public:
    UnrecognisedSQLException(const std::string& sql) : std::runtime_error(
        "Unsupported or invalid SQL token: " + sql + "."
    ) {}
};

// InvalidSQLException for invalid SQL statements.
class InvalidSQLException : public std::runtime_error {
public:
    InvalidSQLException(const std::string& sql) : std::runtime_error(
        "Invalid SQL statement: " + sql + "."
    ) {}
};

} // namespace minisql

#endif // MINISQL_EXCEPTIONS_HPP