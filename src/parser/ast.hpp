#ifndef MINISQL_PARSER_AST_HPP
#define MINISQL_PARSER_AST_HPP

#include <cstddef>
#include <variant>
#include <string>
#include <vector>
#include <optional>

#include "field/field.hpp"

namespace minisql::parser {

using Value = std::variant<double, std::string>;

// 1 condition of a WHERE clause.
struct Condition {
    std::string column;
    enum class Operator { EQ, NEQ, GT, GTE, LT, LTE } op;
    Value value;
};

// 1 modification of a SET clause.
struct Modification {
    std::string column;
    enum class Operator { EQ, ADD, SUB, MUL, DIV } op;
    std::variant<Value, std::string> value;
};

struct CreateAST {
    std::string table;
    std::vector<std::string> columns;
    std::vector<FieldType> types;
    std::vector<std::size_t> sizes;
    std::optional<std::string> primary {std::nullopt};
};

struct SelectAST {
    std::string table;
    std::vector<std::string> columns;
    std::vector<Condition> conditions;
};

struct InsertAST {
    std::string table;
    std::vector<std::string> columns;
    std::vector<std::vector<Value>> values;
};

struct UpdateAST {
    std::string table;
    std::vector<Modification> modifications;
    std::vector<Condition> conditions;
};

struct DeleteAST {
    std::string table;
    std::vector<Condition> conditions;
};

using AST = std::variant<
    CreateAST, SelectAST, InsertAST, UpdateAST, DeleteAST
>;

} // namespace minisql::parser

#endif // MINISQL_PARSER_AST_HPP