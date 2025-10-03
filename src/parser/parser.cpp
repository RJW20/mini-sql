#include "parser/parser.hpp"

#include <cstddef>
#include <string_view>
#include <vector>
#include <string>
#include <cctype>
#include <utility>

#include "parser/ast.hpp"
#include "parser/token.hpp"
#include "field/field.hpp"
#include "exceptions.hpp"
#include "validator/defaults.hpp"

namespace minisql::parser {

namespace {

/* Parser
 * Builds an AST corresponding to the input sql by tokenising. */
class Parser {
public:
    Parser(std::string_view sql) : sql_{sql} { tokenise(sql); }

    AST parse() {
        pos_ = 0;
        switch (peek().type) {
            case TokenType::CREATE: return parse_create();
            case TokenType::SELECT: return parse_select();
            case TokenType::INSERT: return parse_insert();
            case TokenType::UPDATE: return parse_update();
            case TokenType::DELETE: return parse_delete();
            default: raise_exception();
        }
        __builtin_unreachable();
    }

private:
    std::string_view sql_;
    std::vector<Token> tokens_;
    std::size_t pos_;

    void tokenise(std::string_view sql);

    // Token access
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

    // Small object parsing
    std::string parse_identifier() {
        return expect(TokenType::IDENTIFIER).text;
    }
    FieldType parse_type() {
        Token& t = advance();
        if (t.type == TokenType::INT) return FieldType::INT;
        if (t.type == TokenType::REAL) return FieldType::REAL;
        if (t.type == TokenType::TEXT) return FieldType::TEXT;
        raise_exception();
        __builtin_unreachable();
    }
    Value parse_value() {
        Token& t = advance();
        if (t.type == TokenType::NUMBER) return std::stod(t.text);
        if (t.type == TokenType::STRING) return t.text;
        raise_exception();
        __builtin_unreachable();
    }

    Condition parse_condition();
    Modification parse_modification();
    
    CreateAST parse_create();
    SelectAST parse_select();
    InsertAST parse_insert();
    UpdateAST parse_update();
    DeleteAST parse_delete();

    void raise_exception() { throw InvalidSQLException(std::string(sql_)); }
};

// Populate tokens with a sequence of Tokens formed from sql.
void Parser::tokenise(std::string_view sql) {

    std::size_t i = 0;
    while (i < sql.size()) {

        unsigned char c = static_cast<unsigned char>(sql[i]);
        if (std::isspace(c)) {
            i++;
            continue;
        }

        if (c == '(') {
            tokens_.push_back({TokenType::LPAREN, "("});
            i++;
        }
        else if (c == ')') {
            tokens_.push_back({TokenType::RPAREN, ")"});
            i++;
        }
        else if (c == '*') {
            tokens_.push_back({TokenType::STAR, "*"});
            i++;
        }
        else if (c == ',') {
            tokens_.push_back({TokenType::COMMA, ","});
            i++;
        }
        else if (c == ';') {
            tokens_.push_back({TokenType::SEMICOLON, ";"});
            i++;
        }

        else if (std::isalpha(c) || c == '_') {
            std::size_t start = i;
            while (++i < sql.size()) {
                c = static_cast<unsigned char>(sql[i]);
                if (!(std::isalpha(c) || c == '_' || std::isdigit(c))) break;
            }
            std::string_view text = sql.substr(start, i - start);
            TokenType type;
            if (text == "CREATE") type = TokenType::CREATE;
            else if (text == "SELECT") type = TokenType::SELECT;
            else if (text == "INSERT") type = TokenType::INSERT;
            else if (text == "UPDATE") type = TokenType::UPDATE;
            else if (text == "DELETE") type = TokenType::DELETE;
            else if (text == "TABLE") type = TokenType::TABLE;
            else if (text == "INT") type = TokenType::INT;
            else if (text == "REAL") type = TokenType::REAL;
            else if (text == "TEXT") type = TokenType::TEXT;
            else if (text == "PRIMARY") type = TokenType::PRIMARY;
            else if (text == "KEY") type = TokenType::KEY;
            else if (text == "FROM") type = TokenType::FROM;
            else if (text == "INTO") type = TokenType::INTO;
            else if (text == "VALUES") type = TokenType::VALUES;
            else if (text == "SET") type = TokenType::SET;
            else if (text == "WHERE") type = TokenType::WHERE;
            else if (text == "AND") type = TokenType::AND;
            else type = TokenType::IDENTIFIER;
            tokens_.push_back({type, std::string(text)});
        }

        else if (std::isdigit(c)) {
            std::size_t start = i;
            while (++i < sql.size()) {
                c = static_cast<unsigned char>(sql[i]);
                if (!(std::isdigit(c)) && c != '.') break;
            }
            tokens_.push_back(
                {TokenType::NUMBER, std::string(sql.substr(start, i - start))}
            );
        }

        else if (c == '"') {
            std::size_t start = ++i;
            while (i < sql.size() && sql[i] != c) i++;
            tokens_.push_back(
                {TokenType::STRING, std::string{sql.substr(start, i - start)}}
            );
            i++;
        }

        else if (std::string("=!><+-/").find(c) != std::string::npos) {
            tokens_.push_back(
                {TokenType::OPERATOR, std::string{static_cast<char>(c)}}
            );
            i++;
        }

        else throw UnrecognisedSQLException(std::string{static_cast<char>(c)});
    }
}

// Return a Condition built from the immediate tokens.
Condition Parser::parse_condition() {

    Condition condition = {parse_identifier()};

    Token& t = expect(TokenType::OPERATOR);
    if (t.text == "=") condition.op = Condition::Operator::EQ;
    else if (t.text == "!") {
        if (expect(TokenType::OPERATOR).text != "=") raise_exception();
        condition.op = Condition::Operator::NEQ;
    }
    else if (t.text == ">") {
        if (!match(TokenType::OPERATOR))
            condition.op = Condition::Operator::GT;
        else {
            if (peek().text != "=") raise_exception();
            condition.op = Condition::Operator::GTE;
        }
    }
    else if (t.text == "<") {
        if (!match(TokenType::OPERATOR))
            condition.op = Condition::Operator::LT;
        else {
            if (peek().text != "=") raise_exception();
            condition.op = Condition::Operator::LTE;
        }
    }
    else raise_exception();

    condition.value = parse_value();
    return condition;
}

// Return a Modification built from the immediate tokens.
Modification Parser::parse_modification() {
    
    Modification modification = {parse_identifier()};
    if (expect(TokenType::OPERATOR).text != "=") raise_exception();

    if (peek().type == TokenType::IDENTIFIER) {
        std::string column = parse_identifier();
        if (column == modification.column) {
            Token& t = advance();
            if (t.type == TokenType::OPERATOR) {
                if (t.text == "+")
                    modification.op = Modification::Operator::ADD;
                else if (t.text == "-")
                    modification.op = Modification::Operator::SUB;
                else if (t.text == "/")
                    modification.op = Modification::Operator::DIV;
                else raise_exception();
            }
            else if (t.type == TokenType::STAR)
                modification.op = Modification::Operator::MUL;
            else raise_exception();
            modification.value = parse_value();
        }
        else {
            modification.op = Modification::Operator::EQ;
            modification.value = std::move(column);
        }
    }
    else {
        modification.op = Modification::Operator::EQ;
        modification.value = parse_value();
    }

    return modification;
}

// Return a CreateAST built from tokens.
CreateAST Parser::parse_create() {

    expect(TokenType::CREATE);
    expect(TokenType::TABLE);
    CreateAST ast = {parse_identifier()};

    expect(TokenType::LPAREN);
    ast.columns.push_back(parse_identifier());
    FieldType type = parse_type();
    ast.types.push_back(type);
    switch (type) {
        case FieldType::INT:
            ast.sizes.push_back(sizeof(int));
            break;
        case FieldType::REAL:
            ast.sizes.push_back(sizeof(double));
            break;
        case FieldType::TEXT:
            expect(TokenType::LPAREN);
            ast.sizes.push_back(std::stoi(expect(TokenType::NUMBER).text));
            expect(TokenType::RPAREN);
            break;
    }
    while (match(TokenType::COMMA)) {
        if (match(TokenType::PRIMARY)) {
            expect(TokenType::KEY);
            expect(TokenType::LPAREN);
            ast.primary = parse_identifier();
            expect(TokenType::RPAREN);
        }
        else {
            ast.columns.push_back(parse_identifier());
            type = parse_type();
            ast.types.push_back(type);
            switch (type) {
                case FieldType::INT:
                    ast.sizes.push_back(sizeof(int));
                    break;
                case FieldType::REAL:
                    ast.sizes.push_back(sizeof(double));
                    break;
                case FieldType::TEXT:
                    expect(TokenType::LPAREN);
                    ast.sizes.push_back(
                        std::stoi(expect(TokenType::NUMBER).text)
                    );
                    expect(TokenType::RPAREN);
                    break;
            }
        }
    }
    expect(TokenType::RPAREN);
    
    expect(TokenType::SEMICOLON);
    return ast;
}

// Return a SelectAST built from tokens.
SelectAST Parser::parse_select() {

    expect(TokenType::SELECT);

    std::vector<std::string> columns;
    if (match(TokenType::STAR))
        columns.push_back(validator::defaults::ALL_COLUMNS);
    else {
        columns.push_back(parse_identifier());
        while (match(TokenType::COMMA)) columns.push_back(parse_identifier());
    }

    expect(TokenType::FROM);
    SelectAST ast = {parse_identifier(), std::move(columns)};

    if (match(TokenType::WHERE)) {
        ast.conditions.push_back(parse_condition());
        while (match(TokenType::AND))
            ast.conditions.push_back(parse_condition());
    }

    expect(TokenType::SEMICOLON);
    return ast;
}

// Return an InsertASTbuilt from tokens.
InsertAST Parser::parse_insert() {

    expect(TokenType::INSERT);
    expect(TokenType::INTO);
    InsertAST ast = {parse_identifier()};

    if (match(TokenType::LPAREN)) {
        ast.columns.push_back(parse_identifier());
        while (match(TokenType::COMMA))
            ast.columns.push_back(parse_identifier());
        expect(TokenType::RPAREN);
    }
    else ast.columns.push_back(validator::defaults::ALL_COLUMNS);
    
    expect(TokenType::VALUES);

    expect(TokenType::LPAREN);
    std::vector<Value> values;
    values.push_back(parse_value());
    while (match(TokenType::COMMA)) values.push_back(parse_value());
    expect(TokenType::RPAREN);
    ast.values.push_back(std::move(values));
    while (match(TokenType::COMMA)) {
        expect(TokenType::LPAREN);
        values.clear();
        values.push_back(parse_value());
        while (match(TokenType::COMMA)) values.push_back(parse_value());
        expect(TokenType::RPAREN);
        ast.values.push_back(std::move(values));
    }

    expect(TokenType::SEMICOLON);
    return ast;
}

// Return an UpdateAST built from tokens.
UpdateAST Parser::parse_update() {

    expect(TokenType::UPDATE);
    UpdateAST ast = {parse_identifier()};
    expect(TokenType::SET);

    ast.modifications.push_back(parse_modification());
    while (match(TokenType::COMMA))
        ast.modifications.push_back(parse_modification());

    if (match(TokenType::WHERE)) {
        ast.conditions.push_back(parse_condition());
        while (match(TokenType::AND))
            ast.conditions.push_back(parse_condition());
    }

    expect(TokenType::SEMICOLON);
    return ast;
}

// Return a DeleteAST built from tokens.
DeleteAST Parser::parse_delete() {

    expect(TokenType::DELETE);
    expect(TokenType::FROM);
    DeleteAST ast = {parse_identifier()};

    if (match(TokenType::WHERE)) {
        ast.conditions.push_back(parse_condition());
        while (match(TokenType::AND))
            ast.conditions.push_back(parse_condition());
    }

    expect(TokenType::SEMICOLON);
    return ast;
}

} // namespace

AST parse(std::string_view sql) {
    return Parser{sql}.parse();
}

} // namespace minisql::parser