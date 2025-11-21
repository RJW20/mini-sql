#ifndef MINISQL_PLANNER_TABLE_SCAN_HPP
#define MINISQL_PLANNER_TABLE_SCAN_HPP

#include <cfloat>
#include <climits>
#include <memory>
#include <utility>

#include "cursor.hpp"
#include "field/field.hpp"
#include "minisql/varchar.hpp"
#include "planner/iterators/iterator.hpp"
#include "row/row_view.hpp"
#include "row/schema.hpp"

namespace minisql::planner {

// Outputs every Row in a B+ Tree.
class TableScan : public Iterator {
public:
    TableScan(std::unique_ptr<Cursor> cursor, const Schema& schema)
        : cursor_{std::move(cursor)}, schema_{schema} {
            switch (schema_.primary().type) {
                case FieldType::INT:
                    cursor_->open(INT_MIN);
                    break;
                case FieldType::REAL:
                    cursor_->open(DBL_MIN);
                    break;
                case FieldType::TEXT:
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

protected:
    std::unique_ptr<Cursor> cursor_;
    const Schema& schema_;
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_TABLE_SCAN_HPP