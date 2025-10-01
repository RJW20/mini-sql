#ifndef MINISQL_PLANNER_INDEX_SCAN_HPP
#define MINISQL_PLANNER_INDEX_SCAN_HPP

#include <memory>
#include <optional>
#include <utility>
#include <functional>

#include "planner/iterators/table_scan.hpp"
#include "cursor.hpp"
#include "row/schema.hpp"
#include "field/field.hpp"
#include "planner/compiler.hpp"

namespace minisql::planner {

// Outputs Rows in a B+ Tree with primary index between bounds.
class IndexScan : public TableScan {
public:
    IndexScan(
        std::unique_ptr<Cursor> cursor, const Schema& schema,
        std::optional<Field> lb, bool inclusive_lb, std::optional<Field> ub,
        bool inclusive_ub 
    ) : TableScan{std::move(cursor), schema}, lb_{std::move(lb)},
        inclusive_lb_{inclusive_lb}, ub_{std::move(ub)},
        inclusive_ub_{inclusive_ub},
        less_than_{compile_less_than(schema_.primary().type)} {}
        
    void open() override {
        if (lb_) cursor_->open(*lb_);
        else TableScan::open();
     }

    bool next() override {
        if (!cursor_->next()) return false;
        Field key = cursor_->current().primary();
        if (lb_ && !inclusive_lb_ && key == *lb_) {
            if (!cursor_->next()) return false;
            key = cursor_->current().primary();
        }
        if (ub_ && (!less_than_(key, *ub_) || !inclusive_ub_ && key == *ub_))
            return false;
        count_++;
        return true;
    }

private:
    std::optional<Field> lb_;
    bool inclusive_lb_;
    std::optional<Field> ub_;
    bool inclusive_ub_;
    std::function<bool(const Field&, const Field&)> less_than_;
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_INDEX_SCAN_HPP