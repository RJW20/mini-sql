#ifndef MINISQL_TOKEN_HPP
#define MINISQL_TOKEN_HPP

#include <cstdint>

#include "varchar.hpp"

namespace minisql {

enum class TokenType : std::uint8_t {
    LPAREN, RPAREN, STAR, COMMA, SEMICOLON,
    CREATE, SELECT, INSERT, UPDATE, DELETE,
    TABLE, INT, REAL, TEXT, PRIMARY, KEY,
    FROM, INTO, VALUES, SET, WHERE, AND,
    IDENTIFIER, NUMBER, STRING, OPERATOR
};

struct Token {
    TokenType type;
    Varchar text;
};

} // namespace minisql

#endif // MINISQL_TOKEN_HPP