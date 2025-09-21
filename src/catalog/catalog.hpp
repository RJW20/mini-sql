#ifndef MINISQL_CATALOG_HPP
#define MINISQL_CATALOG_HPP

#include <cstdint>
#include <memory>
#include <utility>
#include <unordered_map>

#include "frame_manager/frame_manager.hpp"
#include "varchar.hpp"
#include "row/schema.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "bplus_tree/bplus_tree.hpp"
#include "catalog/table.hpp"

namespace minisql {

/* Catalog
 * Manages a map of Tables. */
class Catalog {
public:
    Catalog(FrameManager& fm) : fm_{fm} {}
    virtual ~Catalog() = default;

    void add_table(
        const Varchar& name, std::unique_ptr<Schema> schema,
        page_id_t root = nullpid, std::uint32_t next_rowid = 0
    ) {
        auto bp_tree = std::make_unique<BPlusTree>(
            fm_, schema->primary().size, schema->row_size(), root
        );
        tables_.emplace(
            name, Table{std::move(bp_tree), std::move(schema), next_rowid}
        );
    }

    const Table* find(const Varchar& name) const {
        auto it = tables_.find(name);
        if (it != tables_.end()) return &(it->second);
        return nullptr;
    }

private:
    FrameManager& fm_;
    std::unordered_map<Varchar, Table> tables_;
};

} // namespace minisql

#endif // MINISQL_CATALOG_HPP