#ifndef MINISQL_PLANNER_HPP
#define MINISQL_PLANNER_HPP

#include <memory>

#include "planner/iterators/iterator.hpp"
#include "parser/query.hpp"
#include "catalog/catalog.hpp"

namespace minisql::planner {

using Plan = std::unique_ptr<Iterator>;
Plan plan(const Query&, Catalog&);

} // namespace minisql::planner

#endif // MINISQL_PLANNER_HPP