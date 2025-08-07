#ifndef MINISQL_PLANNER_VALUES_HPP
#define MINISQL_PLANNER_VALUES_HPP

#include <vector>
#include <utility>

#include "planner/iterators/iterator.hpp"
#include "row/row.hpp"
#include "row/row_view.hpp"

namespace minisql::planner {

// Outputs Rows from a vector.
class Values : public Iterator {
public:
    Values(const std::vector<Row>& rows) : rows_{std::move(rows)} {}

    void open() override {}

    bool next() override {
        if (!(++pos_ < rows_.size())) return false;
        count_++;
        return true;
    }

    RowView current() override { return rows_[pos_].serialise(); }

    void close() override { rows_.clear(); }

private:
    std::vector<Row> rows_;
    std::size_t pos_ {-1};
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_VALUES_HPP