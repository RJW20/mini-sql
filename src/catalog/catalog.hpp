#ifndef MINISQL_CATALOG_HPP
#define MINISQL_CATALOG_HPP

#include <memory>
#include <string>
#include <unordered_map>

#include "catalog/table.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "row/schema.hpp"

namespace minisql {

/* Catalog
 * Defines the interface for a class managing a map of Tables. */
class Catalog {
public:
    virtual ~Catalog() = default;

    virtual void add_table(
        const std::string& name, std::unique_ptr<Schema> schema,
        page_id_t root = nullpid, rowid_t next_rowid = 0
    ) = 0;

    virtual void erase_table(const std::string& name) = 0;

    Table* find_table(const std::string& name) {
        auto it = tables_.find(name);
        if (it != tables_.end()) return &(it->second);
        return nullptr;
    }

    const Table* find_table(const std::string& name) const {
        auto it = tables_.find(name);
        if (it != tables_.end()) return &(it->second);
        return nullptr; 
    }

protected:
    std::unordered_map<std::string, Table> tables_;
};

} // namespace minisql

#endif // MINISQL_CATALOG_HPP