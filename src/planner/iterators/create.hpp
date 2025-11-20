#ifndef MINISQL_PLANNER_CREATE_HPP
#define MINISQL_PLANNER_CREATE_HPP

#include <memory>
#include <string>
#include <utility>

#include "catalog/catalog.hpp"
#include "planner/iterators/iterator.hpp"
#include "row/row_view.hpp"
#include "row/schema.hpp"

namespace minisql::planner {

// Creates a new Table.
class Create : public Iterator {
public:
    Create(
        Catalog& catalog, const std::string& table,
        std::unique_ptr<Schema> schema
    ) : catalog_{catalog}, table_{table}, schema_{std::move(schema)} {}

    bool next() override {
        if (created_) return false;
        catalog_.add_table(table_, std::move(schema_));
        created_ = true;
        return true;
    }

    RowView current() override { return RowView{{}, nullptr}; }

private:
    Catalog& catalog_;
    std::string table_;
    std::unique_ptr<Schema> schema_;
    bool created_ {false};
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_CREATE_HPP