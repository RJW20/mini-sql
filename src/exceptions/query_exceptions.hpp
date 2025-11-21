#ifndef MINISQL_QUERY_EXCEPTIONS_HPP
#define MINISQL_QUERY_EXCEPTIONS_HPP

#include <cstddef>
#include <string>

#include "minisql/exception.hpp"

namespace minisql {

// Base class for exceptions related to invalid queries or statements.
class QueryException : public Exception {
public:
    explicit QueryException(const std::string& msg)
        : Exception("Query error: " + msg) {}
};

// Thrown for invalid query syntax.
class SyntaxException : public QueryException {
public:
    explicit SyntaxException(const std::string& token)
        : QueryException("syntax error near \"" + token + "\"") {}
};

// Base class for exceptions related to tables in queries or statements.
class TableException : public QueryException {
public:
    using QueryException::QueryException;
};

// Thrown when a table name is too large.
class TableNameException : public TableException {
public:
    TableNameException(const std::string& table, std::size_t max_size)
        : TableException(
            "table name \"" + table + "\" cannot exceed " +
            std::to_string(max_size) + " characters"
        ) {}
};

/* Thrown when a referenced table already exists or does not exist within a
 * database. */
class TableExistenceException : public TableException {
public:
    TableExistenceException(const std::string& table, bool exists)
        : TableException(
            "table \"" + table +
            (exists ? "\" already exists" : "\" does not exist")
        ) {}
};

// Thrown when a table's row width is too large.
class TableWidthException : public TableException {
public:
    TableWidthException(
        const std::string& table, std::size_t width, std::size_t max_width
    ) : TableException(
        "table \"" + table + "\" is too wide (maximum width " +
        std::to_string(max_width) + " bytes, got " + std::to_string(width) +
        " bytes)"
    ) {}

};

// Base class for exceptions related to columns in queries or statements.
class ColumnException : public QueryException {
public:
    using QueryException::QueryException;
};

/* Thrown when a referenced column already exists or does not exist within a
 * table. */
class ColumnExistenceException : public ColumnException {
public:
    explicit ColumnExistenceException(const std::string& column, bool exists)
        : ColumnException(
            "column \"" + column +
            (exists ? "\" already exists" : "\" does not exist")
        ) {}
};

// Thrown when a referenced column is reserved.
class ReservedColumnException : public ColumnException {
public:
    explicit ReservedColumnException(const std::string& column)
        : ColumnException("column \"" + column + "\" is reserved") {}
};

// Thrown when an incorrect number of columns are provided.
class ColumnCountException : public ColumnException {
public:
    ColumnCountException(bool fewer)
        : ColumnException(
            std::string("invalid columns list - too ") +
            (fewer ? "few" : "many") + " columns provided"
        ) {}
};

// Thrown when a column is prescribed a value of non-matching type.
class ColumnTypeException : public ColumnException {
public:
    ColumnTypeException(const std::string& column, const std::string& type)
        : ColumnException(
            "column \"" + column + "\" only accepts values of type " + type
        ) {}
};

// Thrown when a column does not support an operation.
class ColumnOperationException : public ColumnException {
public:
    ColumnOperationException(const std::string& column, const std::string& op)
        : ColumnException(
            "column \"" + column + "\" does not support the operator " + op
        ) {}
};

// Thrown when a constant column is modified.
class ConstantColumnException : public ColumnException {
public:
    explicit ConstantColumnException(const std::string& column)
        : ColumnException("column \"" + column + "\" cannot be modified") {}
};

// Thrown when an incorrect number of values are provided.
class ValueCountException : public QueryException {
public:
    ValueCountException(bool fewer)
        : QueryException(
            std::string("invalid values list - too ") +
            (fewer ? "few" : "many") + " values provided"
        ) {}
};

} // namespace minisql

#endif // MINISQL_QUERY_EXCEPTIONS