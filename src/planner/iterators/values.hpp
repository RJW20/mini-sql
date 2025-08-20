#ifndef MINISQL_PLANNER_VALUES_HPP
#define MINISQL_PLANNER_VALUES_HPP

#include <vector>
#include <utility>
#include <limits>

#include "planner/iterators/iterator.hpp"
#include "field.hpp"
#include "row/schema.hpp"
#include "row/row_view.hpp"

namespace minisql::planner {

// Outputs Rows from a vector of vectors of Fields.
class Values : public Iterator {
public:
    Values(
        std::vector<std::vector<Field>> values, std::shared_ptr<Schema> schema
    ) : values_{std::move(values)}, schema_{std::move(schema)} {}

    void open() override {}

    bool next() override {
        if (!(++pos_ < values_.size())) return false;
        count_++;
        return true;
    }

    RowView current() override {
        return Row{std::move(values_[pos_]), schema_}.serialise();
    }

    void close() override { values_.clear(); }

private:
    std::vector<std::vector<Field>> values_;
    std::shared_ptr<Schema> schema_;
    std::size_t pos_ = std::numeric_limits<std::size_t>::max();
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_VALUES_HPP