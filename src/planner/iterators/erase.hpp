#ifndef MINISQL_PLANNER_ERASE_HPP
#define MINISQL_PLANNER_ERASE_HPP

#include <memory>
#include <utility>

#include "planner/iterators/iterator.hpp"
#include "cursor.hpp"
#include "row/row_view.hpp"

namespace minisql::planner {

// Erases Rows from an Iterator from a B+ Tree.
class Erase : public Iterator {
public:
    Erase(std::unique_ptr<Iterator> child, Cursor* cursor)
        : child_{std::move(child)}, cursor_{cursor} {}

    void open() override {
        child_->open();
        cursor_->open();
    }

    bool next() override {
        if (!child_->next()) return false;
        cursor_->seek(child_->current().primary());
        cursor_->erase();
        count_++;
        return true;
    }

    RowView current() override { return child_->current(); }

    void close() override { child_->close(); }

private:
    std::unique_ptr<Iterator> child_;
    Cursor* cursor_;
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_ERASE_HPP