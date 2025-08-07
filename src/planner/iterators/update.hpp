#ifndef MINISQL_PLANNER_UPDATE_HPP
#define MINISQL_PLANNER_UPDATE_HPP

#include <memory>
#include <functional>
#include <utility>

#include "planner/iterators/iterator.hpp"
#include "row/row_view.hpp"
#include "cursor.hpp"

namespace minisql::planner {

// Updates Rows (in-place) from an Iterator.
class Update : public Iterator {
public:
    Update(std::unique_ptr<Iterator> child) : child_{std::move(child)} {}

    void open() override { child_->open(); }

    bool next() override {
        if (!child_->next()) return false;
        modify_(child_->current());
        count_++;
        return true;
    }

    RowView current() override { return child_->current(); }

    void close() override { child_->close(); }

private:
    std::unique_ptr<Iterator> child_;
    std::function<void(const RowView&)> modify_;
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_UPDATE_HPP