#ifndef MINISQL_PLANNER_COMPILER_HPP
#define MINISQL_PLANNER_COMPILER_HPP

#include <functional>

#include "row/field.hpp"
#include "row/schema.hpp"

namespace minisql::planner {

std::function<bool(const Field&, const Field&)> compile_less_than(
    Schema::FieldType type
);

} // namespace minisql::planner

#endif // MINISQL_PLANNER_COMPILER_HPP