#ifndef MINISQL_PARSER_TOKEN_HPP
#define MINISQL_PARSER_TOKEN_HPP

#include <cstdint>
#include <string>

namespace minisql::parser {

enum class TokenType : std::uint8_t {
    LPAREN, RPAREN, STAR, COMMA, SEMICOLON,
    CREATE, SELECT, INSERT, UPDATE, DELETE,
    TABLE, INT, REAL, TEXT, PRIMARY, KEY,
    FROM, INTO, VALUES, SET, WHERE, AND,
    IDENTIFIER, NUMBER, STRING, OPERATOR
};

struct Token {
    TokenType type;
    std::string text;
};

} // namespace minisql::parser

#endif // MINISQL_PARSER_TOKEN_HPP