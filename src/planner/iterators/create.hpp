#ifndef MINISQL_PLANNER_CREATE_HPP
#define MINISQL_PLANNER_CREATE_HPP

#include <memory>
#include <utility>

#include "planner/iterators/iterator.hpp"
#include "table.hpp"
#include "varchar.hpp"
#include "schema.hpp"
#include "row/row_view.hpp"

namespace minisql::planner {

// Creates a new Table.
class Create : public Iterator {
public:
    Create(
        TableCatalog& catalog, Varchar table, std::unique_ptr<Schema> schema
    ) : catalog_{catalog}, table_{std::move(table)}, schema_{std::move(schema)}
        {}

    void open() override {}

    bool next() override {
        if (count_) return false;
        catalog_.create_table(std::move(table_), std::move(schema_));
        count_++;
        return true;
    }

    RowView current() override { return RowView{{}, nullptr}; }

    void close() override {}

private:
    TableCatalog& catalog_;
    Varchar table_;
    std::unique_ptr<Schema> schema_;
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_CREATE_HPP