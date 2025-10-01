#ifndef MINISQL_TABLE_HPP
#define MINISQL_TABLE_HPP

#include <cstdint>
#include <memory>

#include "bplus_tree/bplus_tree.hpp"
#include "row/schema.hpp"

namespace minisql {

using rowid_t = std::uint32_t;

struct Table {
    Table(
        std::unique_ptr<BPlusTree> bp_tree, std::unique_ptr<Schema> schema,
        rowid_t next_rowid
    ) : bp_tree{std::move(bp_tree)}, schema{std::move(schema)}, next_rowid{next_rowid} {}
    
    std::unique_ptr<BPlusTree> bp_tree;
    const std::unique_ptr<Schema> schema;
    rowid_t next_rowid;
};

} // namespace minisql

#endif // MINISQL_TABLE_HPP