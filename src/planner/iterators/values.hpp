#ifndef MINISQL_PLANNER_VALUES_HPP
#define MINISQL_PLANNER_VALUES_HPP

#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "field/field.hpp"
#include "planner/iterators/iterator.hpp"
#include "row/row_view.hpp"
#include "row/schema.hpp"

namespace minisql::planner {

// Outputs Rows from a vector of vectors of Fields.
class Values : public Iterator {
public:
    Values(
        std::vector<std::vector<Field>> values, std::shared_ptr<Schema> schema
    ) : values_{std::move(values)}, schema_{std::move(schema)} {}

    bool next() override {
        if (pos_ + 1 == values_.size()) return false;
        pos_++;
        count_++;
        return true;
    }

    RowView current() override {
        return Row{values_[pos_], schema_}.serialise();
    }

private:
    std::vector<std::vector<Field>> values_;
    std::shared_ptr<Schema> schema_;
    std::size_t pos_ = std::numeric_limits<std::size_t>::max();
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_VALUES_HPP