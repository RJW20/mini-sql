#ifndef MINISQL_PLANNER_UPDATE_HPP
#define MINISQL_PLANNER_UPDATE_HPP

#include <memory>
#include <utility>

#include "planner/compiler.hpp"
#include "planner/iterators/iterator.hpp"
#include "row/row_view.hpp"

namespace minisql::planner {

// Updates Rows (in-place) from an Iterator.
class Update : public Iterator {
public:
    Update(std::unique_ptr<Iterator> child, Modifier modifier)
        : child_{std::move(child)}, modifier_{std::move(modifier)} {}

    bool next() override {
        if (!child_->next()) return false;
        RowView current = child_->current();
        modifier_(current);
        count_++;
        return true;
    }

    RowView current() override { return child_->current(); }

private:
    std::unique_ptr<Iterator> child_;
    Modifier modifier_;
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_UPDATE_HPP