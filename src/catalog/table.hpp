#ifndef MINISQL_TABLE_HPP
#define MINISQL_TABLE_HPP

#include <cstdint>
#include <memory>

#include "bplus_tree/bplus_tree.hpp"
#include "row/schema.hpp"

namespace minisql {

struct Table {
    std::unique_ptr<BPlusTree> bp_tree;
    const std::unique_ptr<Schema> schema;
    std::uint32_t next_rowid;
};

} // namespace minisql

#endif // MINISQL_TABLE_HPP