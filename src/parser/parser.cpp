#include "parser/parser.hpp"

#include <cstddef>
#include <string_view>
#include <string>
#include <cctype>
#include <utility>
#include <vector>

#include "parser/query.hpp"
#include "parser/token.hpp"
#include "exceptions.hpp"
#include "varchar.hpp"
#include "field.hpp"

namespace minisql {

// Return a Query built from sql.
Query Parser::parse(std::string_view sql) {
    Parser parser{sql};
    switch (parser.peek().type) {
        case TokenType::CREATE: return parser.parse_create();
        case TokenType::SELECT: return parser.parse_select();
        case TokenType::INSERT: return parser.parse_insert();
        case TokenType::UPDATE: return parser.parse_update();
        case TokenType::DELETE: return parser.parse_delete();
        default: throw InvalidSQLException(std::string(sql));
    }
}

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
            tokens_.push_back({TokenType::LPAREN, '('});
            i++;
        }
        else if (c == ')') {
            tokens_.push_back({TokenType::RPAREN, ')'});
            i++;
        }
        else if (c == '*') {
            tokens_.push_back({TokenType::STAR, '*'});
            i++;
        }
        else if (c == ',') {
            tokens_.push_back({TokenType::COMMA, ','});
            i++;
        }
        else if (c == ';') {
            tokens_.push_back({TokenType::SEMICOLON, ';'});
            i++;
        }

        else if (std::isalpha(c) || c == '_') {
            std::size_t start = i;
            while (++i < sql.size()) {
                c = static_cast<unsigned char>(sql[i]);
                if (!(std::isalpha(c) || c == '_' || std::isdigit(c)))
                    break;
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
            tokens_.push_back({type, {text.data(), text.size()}});
        }

        else if (std::isdigit(c)) {
            std::size_t start = i;
            while (++i < sql.size()) {
                c = static_cast<unsigned char>(sql[i]);
                if (!(std::isdigit(c) || c == ','))
                    break;
            }
            std::string_view number = sql.substr(start, i - start);
            std::string clean_number;
            for (char d : number) if (d != ',') clean_number.push_back(c);
            tokens_.push_back(
                {TokenType::NUMBER, {clean_number.data(), clean_number.size()}}
            );
        }

        else if (c == '\'' || c == '"') {
            std::size_t start = ++i;
            while (i < sql.size() && sql[i] != c) i++;
            std::string_view text = sql.substr(start, i - start);
            tokens_.push_back({TokenType::STRING, {text.data(), text.size()}});
            i++;
        }

        else if (std::string("=!><+-/").find(c) != std::string::npos) {
            tokens_.push_back({TokenType::OPERATOR, c});
            i++;
        }

        else throw UnrecognisedSQLException(
            std::string(1, static_cast<char>(c))
        );
    }
}

// Return a Condition built from the immediate tokens.
Condition Parser::parse_condition() {

    Condition condition = {parse_identifier()};

    Token& t = expect(TokenType::OPERATOR);
    if (t.text == '=') condition.op = Condition::Operator::EQ;
    else if (t.text == '!') {
        if (expect(TokenType::OPERATOR).text != '=') raise_exception();
        condition.op = Condition::Operator::NEQ;
    }
    else if (t.text == '>') {
        if (!match(TokenType::OPERATOR))
            condition.op = Condition::Operator::GT;
        else {
            if (peek().text != '=') raise_exception();
            condition.op = Condition::Operator::GTE;
        }
    }
    else if (t.text == '<') {
        if (!match(TokenType::OPERATOR))
            condition.op = Condition::Operator::LT;
        else {
            if (peek().text != '=') raise_exception();
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
    if (expect(TokenType::OPERATOR).text != '=') raise_exception();

    if (peek().type == TokenType::IDENTIFIER) {
        Varchar column = parse_identifier();
        if (column == modification.column) {
            Token& t = advance();
            if (t.type == TokenType::OPERATOR) {
                if (t.text == '+')
                    modification.op = Modification::Operator::ADD;
                else if (t.text == '-')
                    modification.op = Modification::Operator::SUB;
                else if (t.text == '/')
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

// Return a CreateQuery built from tokens.
CreateQuery Parser::parse_create() {

    expect(TokenType::CREATE);
    expect(TokenType::TABLE);
    CreateQuery query = {parse_identifier(), {}, {}, {}, {"rowid", 5}};

    expect(TokenType::LPAREN);
    query.columns.push_back(parse_identifier());
    FieldType type = parse_type();
    query.types.push_back(type);
    switch (type) {
        case FieldType::INT:
            query.sizes.push_back(sizeof(int));
            break;
        case FieldType::REAL:
            query.sizes.push_back(sizeof(double));
            break;
        case FieldType::TEXT:
            expect(TokenType::LPAREN);
            query.sizes.push_back(
                std::stoi(expect(TokenType::NUMBER).text.data())
            );
            expect(TokenType::RPAREN);
            break;
    }
    while (match(TokenType::COMMA)) {
        if (match(TokenType::PRIMARY)) {
            expect(TokenType::KEY);
            expect(TokenType::LPAREN);
            query.primary = parse_identifier();
            expect(TokenType::RPAREN);
        }
        else {
            query.columns.push_back(parse_identifier());
            type = parse_type();
            query.types.push_back(type);
            switch (type) {
                case FieldType::INT:
                    query.sizes.push_back(sizeof(int));
                    break;
                case FieldType::REAL:
                    query.sizes.push_back(sizeof(double));
                    break;
                case FieldType::TEXT:
                    expect(TokenType::LPAREN);
                    query.sizes.push_back(
                        std::stoi(expect(TokenType::NUMBER).text.data())
                    );
                    expect(TokenType::RPAREN);
                    break;
            }
        }
    }
    expect(TokenType::RPAREN);
    
    expect(TokenType::SEMICOLON);
    return query;
}

// Return a SelectQuery built from tokens.
SelectQuery Parser::parse_select() {

    expect(TokenType::SELECT);

    std::vector<Varchar> columns;
    if (match(TokenType::STAR)) columns.push_back('*');
    else {
        columns.push_back(parse_identifier());
        while (match(TokenType::COMMA)) columns.push_back(parse_identifier());
    }

    expect(TokenType::FROM);
    SelectQuery query = {parse_identifier(), std::move(columns)};

    if (match(TokenType::WHERE)) {
        query.conditions.push_back(parse_condition());
        while (match(TokenType::AND))
            query.conditions.push_back(parse_condition());
    }

    expect(TokenType::SEMICOLON);
    return query;
}

// Return an InsertQuery built from tokens.
InsertQuery Parser::parse_insert() {

    expect(TokenType::INSERT);
    expect(TokenType::INTO);
    InsertQuery query = {parse_identifier()};

    if (match(TokenType::LPAREN)) {
        query.columns.push_back(parse_identifier());
        while (match(TokenType::COMMA))
            query.columns.push_back(parse_identifier());
        expect(TokenType::RPAREN);
    }
    else query.columns.push_back('*');
    
    expect(TokenType::VALUES);

    expect(TokenType::LPAREN);
    std::vector<Field> values;
    values.push_back(parse_value());
    while (match(TokenType::COMMA)) values.push_back(parse_value());
    expect(TokenType::RPAREN);
    query.values.push_back(std::move(values));
    while (match(TokenType::COMMA)) {
        expect(TokenType::LPAREN);
        values.clear();
        values.push_back(parse_value());
        while (match(TokenType::COMMA)) values.push_back(parse_value());
        expect(TokenType::RPAREN);
        query.values.push_back(std::move(values));
    }

    expect(TokenType::SEMICOLON);
    return query;
}

// Return an UpdateQuery built from tokens.
UpdateQuery Parser::parse_update() {

    expect(TokenType::UPDATE);
    UpdateQuery query = {parse_identifier()};
    expect(TokenType::SET);

    query.modifications.push_back(parse_modification());
    while (match(TokenType::COMMA))
        query.modifications.push_back(parse_modification());

    if (match(TokenType::WHERE)) {
        query.conditions.push_back(parse_condition());
        while (match(TokenType::AND))
            query.conditions.push_back(parse_condition());
    }

    expect(TokenType::SEMICOLON);
    return query;
}

// Return a DeleteQuery built from tokens.
DeleteQuery Parser::parse_delete() {

    expect(TokenType::DELETE);
    expect(TokenType::FROM);
    DeleteQuery query = {parse_identifier()};

    if (match(TokenType::WHERE)) {
        query.conditions.push_back(parse_condition());
        while (match(TokenType::AND))
            query.conditions.push_back(parse_condition());
    }

    expect(TokenType::SEMICOLON);
    return query;
}

} // namespace minisql