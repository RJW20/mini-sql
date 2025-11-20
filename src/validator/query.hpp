#ifndef MINISQL_VALIDATOR_QUERY_HPP
#define MINISQL_VALIDATOR_QUERY_HPP

#include <cstddef>
#include <string>
#include <variant>
#include <vector>

#include "field/field.hpp"

namespace minisql::validator {

// 1 condition of a WHERE clause.
struct Condition {
    std::string column;
    enum class Operator { EQ, NEQ, GT, GTE, LT, LTE } op;
    Field value;
};

// 1 modification of a SET clause.
struct Modification {
    std::string column;
    enum class Operator { EQ, ADD, SUB, MUL, DIV } op;
    std::variant<Field, std::string> value;
};

struct CreateQuery {
    std::string table;
    std::vector<std::string> columns;
    std::vector<FieldType> types;
    std::vector<std::size_t> sizes;
    std::string primary;
};

struct SelectQuery {
    std::string table;
    std::vector<std::string> columns;
    std::vector<Condition> conditions;
};

struct InsertQuery {
    std::string table;
    std::vector<std::string> columns;
    std::vector<std::vector<Field>> values;
};

struct UpdateQuery {
    std::string table;
    std::vector<Modification> modifications;
    std::vector<Condition> conditions;
};

struct DeleteQuery {
    std::string table;
    std::vector<Condition> conditions;
};

struct DropQuery {
    std::string table;
};

using Query = std::variant<
    CreateQuery, SelectQuery, InsertQuery, UpdateQuery, DeleteQuery, DropQuery
>;

} // namespace minisql::validator

#endif // MINISQL_QUERY_HPP