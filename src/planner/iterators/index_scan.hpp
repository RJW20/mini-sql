#ifndef MINISQL_PLANNER_INDEX_SCAN_HPP
#define MINISQL_PLANNER_INDEX_SCAN_HPP

#include <cfloat>
#include <climits>
#include <functional>
#include <memory>
#include <optional>
#include <utility>

#include "cursor.hpp"
#include "field/field.hpp"
#include "field/varchar.hpp"
#include "planner/compiler.hpp"
#include "planner/iterators/iterator.hpp"
#include "row/schema.hpp"

namespace minisql::planner {

// Outputs Rows in a B+ Tree with primary index between bounds.
class IndexScan : public Iterator {
public:
    IndexScan(
        std::unique_ptr<Cursor> cursor, const Schema& schema,
        std::optional<Field> lb, bool inclusive_lb, std::optional<Field> ub,
        bool inclusive_ub 
    ) : cursor_{std::move(cursor)}, schema_{schema}, lb_{std::move(lb)},
        inclusive_lb_{inclusive_lb}, ub_{std::move(ub)},
        inclusive_ub_{inclusive_ub},
        less_than_{compile_less_than(schema_.primary().type)} {
            if (lb_) cursor_->open(*lb_);
            else {
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
        }

    bool next() override {
        if (!cursor_->next()) return false;
        Field key = cursor_->current().primary();
        if (lb_ && !inclusive_lb_ && key == *lb_) {
            if (!cursor_->next()) return false;
            key = cursor_->current().primary();
        }
        if (ub_ && !(less_than_(key, *ub_) || inclusive_ub_ && key == *ub_))
            return false;
        count_++;
        return true;
    }

    RowView current() override { return cursor_->current(); }

private:
    std::unique_ptr<Cursor> cursor_;
    const Schema& schema_;
    std::optional<Field> lb_;
    bool inclusive_lb_;
    std::optional<Field> ub_;
    bool inclusive_ub_;
    std::function<bool(const Field&, const Field&)> less_than_;
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_INDEX_SCAN_HPP