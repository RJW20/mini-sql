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
        const Varchar& name, std::unique_ptr<Schema> schema, page_id_t root,
        std::uint32_t next_rowid
    ) {
        auto bp_tree = std::make_unique<BPlusTree>(
            fm_, schema->primary().size, schema->row_size(), root
        );
        tables_.emplace(
            name, Table{std::move(schema), std::move(bp_tree), next_rowid}
        );
    }

    void new_table(const Varchar& name, std::unique_ptr<Schema> schema) {
        add_table(name, std::move(schema), nullpid, 0);
    }

    Table& table(const Varchar& name) { return tables_.at(name); }

private:
    FrameManager& fm_;
    std::unordered_map<Varchar, Table> tables_;
};

} // namespace minisql

#endif // MINISQL_CATALOG_HPP