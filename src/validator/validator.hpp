#ifndef MINISQL_VALIDATOR_HPP
#define MINISQL_VALIDATOR_HPP

#include "catalog/catalog.hpp"
#include "parser/ast.hpp"
#include "validator/query.hpp"

namespace minisql::validator {

Query validate(const parser::AST&, Catalog&, bool);

} // namespace minisql::validator

#endif // MINISQL_VALIDATOR_HPP