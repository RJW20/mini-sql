#ifndef MINISQL_PLANNER_INSERT_HPP
#define MINISQL_PLANNER_INSERT_HPP

#include <memory>
#include <utility>

#include "planner/iterators/iterator.hpp"
#include "cursor.hpp"
#include "row/row_view.hpp"

namespace minisql::planner {

// Inserts Rows from an Iterator into a B+ Tree.
class Insert : public Iterator {
public:
    Insert(
        std::unique_ptr<Iterator> child, std::unique_ptr<Cursor> cursor
    ) : child_{std::move(child)}, cursor_{std::move(cursor)} {}

    void open() override {
        child_->open();
        cursor_->open();
    }

    bool next() override {
        if (!child_->next()) return false;
        RowView rv = child_->current();
        cursor_->seek(rv.primary());
        cursor_->insert(rv);
        count_++;
        return true;
    }

    RowView current() override { return cursor_->current(); }

    void close() override {
        child_->close();
        cursor_->close();
    }

private:
    std::unique_ptr<Iterator> child_;
    std::unique_ptr<Cursor> cursor_;
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_INSERT_HPP