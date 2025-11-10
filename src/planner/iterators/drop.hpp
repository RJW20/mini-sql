#ifndef MINISQL_PLANNER_DROP_HPP
#define MINISQL_PLANNER_DROP_HPP

#include <string>

#include "planner/iterators/iterator.hpp"
#include "catalog/catalog.hpp"
#include "row/row_view.hpp"

namespace minisql::planner {

// Drops a Table.
class Drop : public Iterator {
public:
    Drop(Catalog& catalog, const std::string& table)
        : catalog_{catalog}, table_{table}{}

    bool next() override {
        if (dropped_) return false;
        catalog_.erase_table(table_);
        dropped_ = true;
        return true;
    }

    RowView current() override { return RowView{{}, nullptr}; }

private:
    Catalog& catalog_;
    std::string table_;
    bool dropped_ {false};
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_DROP_HPP