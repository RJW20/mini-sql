#ifndef MINISQL_PLANNER_FILTER_HPP
#define MINISQL_PLANNER_FILTER_HPP

#include <memory>
#include <utility>

#include "planner/iterators/iterator.hpp"
#include "planner/compiler.hpp"
#include "row/row_view.hpp"

namespace minisql::planner {

// Outputs Rows from an Iterator that pass a Predicate.
class Filter : public Iterator {
public:
    Filter(std::unique_ptr<Iterator> child, const Predicate& predicate)
        : child_{std::move(child)}, predicate_{std::move(predicate)} {}

    void open() override { child_->open(); }

    bool next() override {
        while (child_->next())
            if (predicate_(child_->current())) {
                count_++;
                return true;
            }
        return false;
    }

    RowView current() override { return child_->current(); }

    void close() override { child_->close(); }

private:
    std::unique_ptr<Iterator> child_;
    Predicate predicate_;
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_FILTER_HPP