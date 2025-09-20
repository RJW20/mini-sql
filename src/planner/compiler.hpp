#ifndef MINISQL_PLANNER_COMPILER_HPP
#define MINISQL_PLANNER_COMPILER_HPP

#include <functional>
#include <vector>

#include "field.hpp"
#include "row/row_view.hpp"
#include "parser/query.hpp"
#include "row/schema.hpp"

namespace minisql::planner {

std::function<bool(const Field&, const Field&)> compile_less_than(FieldType);

using Predicate = std::function<bool(const RowView&)>;
Predicate compile(const std::vector<Condition>&, const Schema*);

using Modifier = std::function<void(RowView&)>;
Modifier compile(const std::vector<Modification>&, const Schema*);

} // namespace minisql::planner

#endif // MINISQL_PLANNER_COMPILER_HPP