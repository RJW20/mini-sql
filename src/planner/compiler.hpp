#ifndef MINISQL_PLANNER_COMPILER_HPP
#define MINISQL_PLANNER_COMPILER_HPP

#include <functional>

#include "field.hpp"
#include "row/row_view.hpp"

namespace minisql::planner {

std::function<bool(const Field&, const Field&)> compile_less_than(FieldType);

using Predicate = std::function<bool(const RowView&)>;

using Modifier = std::function<void(RowView&)>;

} // namespace minisql::planner

#endif // MINISQL_PLANNER_COMPILER_HPP