#ifndef MINISQL_PLANNER_HPP
#define MINISQL_PLANNER_HPP

#include <memory>

#include "planner/iterators/iterator.hpp"
#include "parser/query.hpp"
#include "table.hpp"

namespace minisql::planner {

using Plan = std::unique_ptr<Iterator>;
Plan plan(const Query&, TableCatalog&);

} // namespace minisql::planner

#endif // MINISQL_PLANNER_HPP