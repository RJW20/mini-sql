#ifndef MINISQL_QUERY_HPP
#define MINISQL_QUERY_HPP

#include <cstddef>
#include <variant>
#include <vector>

#include "varchar.hpp"
#include "field.hpp"

namespace minisql {

// AST for 1 condition of a WHERE clause.
struct Condition {
    Varchar column;
    enum class Operator { EQ, NEQ, GT, GTE, LT, LTE } op;
    Field value;
};

// AST for 1 modification of a SET clause.
struct Modification {
    Varchar column;
    enum class Operator { EQ, ADD, SUB, MUL, DIV } op;
    std::variant<Field, Varchar> value;
};

struct CreateQuery {
    Varchar table;
    std::vector<Varchar> columns;
    std::vector<FieldType> types;
    std::vector<std::size_t> sizes;
};

struct SelectQuery {
    Varchar table;
    std::vector<Varchar> columns;
    std::vector<Condition> conditions;
};

struct InsertQuery {
    Varchar table;
    std::vector<Varchar> columns;
    std::vector<std::vector<Field>> values;
};

struct UpdateQuery {
    Varchar table;
    std::vector<Condition> conditions;
    std::vector<Modification> modifications;
};

struct DeleteQuery {
    Varchar table;
    std::vector<Condition> conditions;
};

using Query = std::variant<
    CreateQuery, SelectQuery, InsertQuery, UpdateQuery, DeleteQuery
>;

} // namespace minisql

#endif // MINISQL_QUERY_HPP