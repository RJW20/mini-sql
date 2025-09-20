#ifndef MINISQL_TABLE_HPP
#define MINISQL_TABLE_HPP

#include <cstdint>
#include <memory>

#include "row/schema.hpp"
#include "bplus_tree/bplus_tree.hpp"

namespace minisql {

struct Table {
    const std::unique_ptr<Schema> schema;
    std::unique_ptr<BPlusTree> bp_tree;
    std::uint32_t next_rowid;
};

} // namespace minisql

#endif // MINISQL_TABLE_HPP