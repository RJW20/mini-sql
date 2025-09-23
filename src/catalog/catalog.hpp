#ifndef MINISQL_CATALOG_HPP
#define MINISQL_CATALOG_HPP

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "varchar.hpp"
#include "row/schema.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "catalog/table.hpp"

namespace minisql {

/* Catalog
 * Defines the interface for a class managing a map of Tables. */
class Catalog {
public:
    virtual ~Catalog() = default;

    virtual void add_table(
        const Varchar& name, std::unique_ptr<Schema> schema,
        page_id_t root = nullpid, std::uint32_t next_rowid = 0
    ) = 0;

    virtual const Table* find_table(const Varchar& name) const = 0;

protected:
    std::unordered_map<Varchar, Table> tables_;
};

} // namespace minisql

#endif // MINISQL_CATALOG_HPP