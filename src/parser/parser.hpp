#ifndef MINISQL_PARSER_HPP
#define MINISQL_PARSER_HPP

#include <cstddef>
#include <string_view>
#include <vector>
#include <string>
#include <utility>

#include "parser/query.hpp"
#include "parser/token.hpp"
#include "varchar.hpp"
#include "field.hpp"
#include "exceptions.hpp"

namespace minisql {

/* Parser
 * Builds a Query corresponding to the input sql by tokenising. */
class Parser {
public:
    static Query parse(std::string_view sql);

private:
    Parser(std::string_view sql) : sql_{sql}, pos_{0} { tokenise(sql); }

    std::string_view sql_;
    std::vector<Token> tokens_;
    std::size_t pos_;

    void tokenise(std::string_view sql);

    const Token& peek() const { return tokens_[pos_]; }
    Token& advance() { return tokens_[pos_++]; }
    bool match(TokenType t) {
        if (peek().type != t) return false;  
        advance();
        return true;
    }
    Token& expect(TokenType t) {
        if (peek().type != t) raise_exception();
        return advance();
    }

    Varchar parse_identifier() {
        return std::move(expect(TokenType::IDENTIFIER).text);
    }
    FieldType parse_type() {
        Token& t = advance();
        if (t.type == TokenType::INT) return FieldType::INT;
        if (t.type == TokenType::REAL) return FieldType::REAL;
        if (t.type == TokenType::TEXT) return FieldType::TEXT;
        raise_exception();
        __builtin_unreachable();
    }
    Field parse_value() {
        Token& t = advance();
        if (t.type == TokenType::NUMBER) return std::stod(t.text.data());
        if (t.type == TokenType::STRING) return std::move(t.text);
        raise_exception();
        __builtin_unreachable();
    }

    void raise_exception() { throw InvalidSQLException(std::string(sql_)); }

    Condition parse_condition();
    Modification parse_modification();
    
    CreateQuery parse_create();
    SelectQuery parse_select();
    InsertQuery parse_insert();
    UpdateQuery parse_update();
    DeleteQuery parse_delete();
};

} // namespace minisql

#endif // MINISQL_PARSER_HPP