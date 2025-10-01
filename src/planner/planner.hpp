#ifndef MINISQL_PLANNER_HPP
#define MINISQL_PLANNER_HPP

#include <memory>

#include "planner/iterators/iterator.hpp"
#include "validator/query.hpp"
#include "catalog/catalog.hpp"

namespace minisql::planner {

using Plan = std::unique_ptr<Iterator>;
Plan plan(const validator::Query&, Catalog&);

} // namespace minisql::planner

#endif // MINISQL_PLANNER_HPP