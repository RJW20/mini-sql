#ifndef MINISQL_ROW_SET_HPP
#define MINISQL_ROW_SET_HPP

#include <utility>

#include "planner/planner.hpp"
#include "row_set/row_iterator.hpp"

namespace minisql {

/* RowSet
 * Wrapper over a Plan. */
class RowSet {
public:
    explicit RowSet(planner::Plan plan) : plan_{std::move(plan)} {
        plan_->open();
    }

    RowIterator begin() { return RowIterator{plan_.get()}; }
    RowIterator end() { return RowIterator{}; }

    bool next() { return (plan_ && plan_->next()); }
    Row current() const { return plan_->current().deserialise(); }

private:
    planner::Plan plan_;
};

} // namespace minisql

#endif // MINISQL_ROW_SET_HPP