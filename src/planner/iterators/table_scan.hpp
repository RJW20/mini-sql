#ifndef MINISQL_PLANNER_TABLE_SCAN_HPP
#define MINISQL_PLANNER_TABLE_SCAN_HPP

#include <memory>
#include <utility>
#include <climits>
#include <cfloat>

#include "planner/iterators/iterator.hpp"
#include "cursor.hpp"
#include "row/schema.hpp"
#include "varchar.hpp"
#include "row/row_view.hpp"

namespace minisql::planner {

// Outputs every Row in a B+ Tree.
class TableScan : public Iterator {
public:
    TableScan(std::unique_ptr<Cursor> cursor, const Schema* schema)
        : cursor_{std::move(cursor)}, schema_{schema} {}

    void open() override {
        switch (schema_->primary().type) {
            case Schema::FieldType::INT:
                cursor_->open(INT_MIN);
                break;
            case Schema::FieldType::REAL:
                cursor_->open(DBL_MIN);
                break;
            case Schema::FieldType::TEXT:
                cursor_->open(VCHR_MIN);
                break;
        }
    }

    bool next() override {
        if (!cursor_->next()) return false;
        count_++;
        return true;
    }

    RowView current() override { return cursor_->current(); }

    void close() override { cursor_->close(); }

protected:
    std::unique_ptr<Cursor> cursor_;
    const Schema* schema_;
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_TABLE_SCAN_HPP