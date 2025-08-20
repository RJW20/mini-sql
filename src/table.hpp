#ifndef MINISQL_TABLE_HPP
#define MINISQL_TABLE_HPP

#include <memory>

#include "bplus_tree/bplus_tree.hpp"
#include "row/schema.hpp"
#include "varchar.hpp"

namespace minisql {

struct Table {
    BPlusTree bp_tree;
    const Schema schema;
};

class TableCatalog {
public:
    ~TableCatalog() = default;
    virtual void create_table(
        Varchar name, std::unique_ptr<Schema> schema
    ) = 0;
    virtual const Table* table(const Varchar& name) const = 0;
};

} // namespace minisql

#endif // MINISQL_TABLE_HPP