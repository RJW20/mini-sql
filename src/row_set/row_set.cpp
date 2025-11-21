#include "minisql/row_set.hpp"

#include <utility>

#include "minisql/row.hpp"
#include "minisql/row_iterator.hpp"
#include "planner/planner.hpp"

namespace minisql {

RowSet::RowSet(planner::Plan plan) : plan_{std::move(plan)} {}
RowSet::~RowSet() {}

// STL-style iteration
RowIterator RowSet::begin() { return RowIterator{plan_.get()}; }
RowIterator RowSet::end() { return RowIterator{}; }

// Manual iteration
bool RowSet::next() { return plan_->next(); }
Row RowSet::current() const { return plan_->current().deserialise(); }

} // namespace minisql