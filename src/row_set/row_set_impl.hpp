#ifndef MINISQL_ROW_SET_IMPL_HPP
#define MINISQL_ROW_SET_IMPL_HPP

#include "minisql/row.hpp"
#include "minisql/row_set.hpp"
#include "minisql/row_iterator.hpp"
#include "planner/planner.hpp"

namespace minisql {

/* RowSet Implementation
 * Acts as a wrapper over a Plan. */
class RowSet::Impl {
public:
    RowIterator begin() { return RowIterator{plan.get()}; }

    bool next() { return plan && plan->next(); }
    Row current() { return plan->current().deserialise(); }

    planner::Plan plan {nullptr};
};

} // namespace minisql

#endif // MINISQL_ROW_SET_IMPL_HPP