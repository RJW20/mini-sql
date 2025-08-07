#ifndef MINISQL_PLANNER_PROJECT_HPP
#define MINISQL_PLANNER_PROJECT_HPP

#include <memory>
#include <utility>

#include "planner/iterators/iterator.hpp"
#include "row/schema.hpp"
#include "row/row_view.hpp"

namespace minisql::planner {

// Outputs projections of Rows from an Iterator.
class Project : public Iterator {
public:
    Project(std::unique_ptr<Iterator> child, std::unique_ptr<Schema> schema)
        : child_{std::move(child)}, projected_schema_{std::move(schema)} {}

    void open() override { child_->open(); }

    bool next() override {
        if (!child_->next()) return false;
        count_++;
        return true;
     }

    RowView current() override {
        return RowView{child_->current().data(), projected_schema_};
    }

    void close() override { child_->close(); }

private:
    std::unique_ptr<Iterator> child_;
    std::shared_ptr<Schema> projected_schema_;
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_PROJECT_HPP