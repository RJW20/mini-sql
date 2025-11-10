#ifndef MINISQL_PLANNER_CREATE_HPP
#define MINISQL_PLANNER_CREATE_HPP

#include <string>
#include <memory>
#include <utility>

#include "planner/iterators/iterator.hpp"
#include "catalog/catalog.hpp"
#include "row/schema.hpp"
#include "row/row_view.hpp"

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