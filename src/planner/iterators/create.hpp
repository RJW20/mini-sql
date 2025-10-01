#ifndef MINISQL_PLANNER_CREATE_HPP
#define MINISQL_PLANNER_CREATE_HPP

#include <string>
#include <memory>
#include <utility>

#include "planner/iterators/iterator.hpp"
#include "catalog/table.hpp"
#include "row/schema.hpp"
#include "row/row_view.hpp"

namespace minisql::planner {

// Creates a new Table.
class Create : public Iterator {
public:
    Create(
        Catalog& catalog, std::string& table, std::unique_ptr<Schema> schema
    ) : catalog_{catalog}, table_{table}, schema_{std::move(schema)}
        {}

    void open() override {}

    bool next() override {
        if (count_) return false;
        catalog_.add_table(table_, std::move(schema_));
        count_++;
        return true;
    }

    RowView current() override { return RowView{{}, nullptr}; }

    void close() override {}

private:
    Catalog& catalog_;
    std::string table_;
    std::unique_ptr<Schema> schema_;
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_CREATE_HPP