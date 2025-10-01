#ifndef MINISQL_PARSER_HPP
#define MINISQL_PARSER_HPP

#include <string_view>

#include "parser/ast.hpp"

namespace minisql::parser {

AST parse(std::string_view sql);

} // namespace minisql::parser

#endif // MINISQL_PARSER_HPP