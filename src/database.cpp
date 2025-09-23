#include "database.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>
#include <memory>
#include <utility>
#include <tuple>

#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"
#include "byte_io.hpp"
#include "frame_manager/frame_manager.hpp"
#include "varchar.hpp"
#include "row/schema.hpp"
#include "bplus_tree/bplus_tree.hpp"
#include "catalog/table.hpp"

namespace minisql {

// Create or read the Database header and initialise fm_ and catalog_.
Database::Database(const std::filesystem::path& path) {
    page_id_t page_count {0}, first_free_list_block {nullpid};
    if (!std::filesystem::exists(path)) {
        file_.open(path, std::ios::out);
        if (!file_) throw std::ios_base::failure(
            "Failed to open database: " + path.string()
        );
        file_.close();
        file_.open(path, std::ios::in | std::ios::out | std::ios::binary);
        master_root_ = nullpid;
    }
    else {
        file_.open(path, std::ios::in | std::ios::out | std::ios::binary);
        if (!file_) throw std::ios_base::failure(
            "Failed to open database: " + path.string()
        );
        std::vector<std::byte> db_header{DatabaseHeader::SIZE};
        file_.seekg(0);
        file_.read(
            reinterpret_cast<char*>(db_header.data()), DatabaseHeader::SIZE
        );
        page_count = byte_io::view<page_id_t>(
            db_header, DatabaseHeader::PAGE_COUNT_OFFSET
        );
        first_free_list_block = byte_io::view<page_id_t>(
            db_header, DatabaseHeader::FIRST_FREE_LIST_BLOCK_OFFSET
        );
        master_root_ = byte_io::view<page_id_t>(
            db_header, DatabaseHeader::MASTER_ROOT_OFFSET
        );
    }
    fm_ = std::make_unique<FrameManager>(
        file_, DatabaseHeader::SIZE, PAGE_SIZE_, page_count, CACHE_CAPACITY_,
        first_free_list_block
    );
}

// Flush any dirty pages and update the DatabaseHeader.
Database::~Database() {
    fm_->flush_all();
    std::vector<std::byte> db_header{DatabaseHeader::SIZE};
    byte_io::write<page_id_t>(
        db_header, DatabaseHeader::PAGE_COUNT_OFFSET, fm_->page_count()
    );
    byte_io::write<page_id_t>(
        db_header, DatabaseHeader::FIRST_FREE_LIST_BLOCK_OFFSET,
        fm_->first_free_list_block()
    );
    byte_io::write<page_id_t>(
        db_header, DatabaseHeader::MASTER_ROOT_OFFSET, master_root_
    );
    file_.seekp(0);
    file_.write(
        reinterpret_cast<const char*>(db_header.data()), DatabaseHeader::SIZE
    );
    file_.flush();
}

// Construct a Table in the Catalog with given name.
void Database::add_table(
    const Varchar& name, std::unique_ptr<Schema> schema, page_id_t root,
    std::uint32_t next_rowid
) {
    auto bp_tree = std::make_unique<BPlusTree>(
        fm_.get(), schema->primary().size, schema->row_size(), root
    );
    tables_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(name),
        std::forward_as_tuple(std::move(bp_tree), std::move(schema), next_rowid)
    );
}

/* Return a pointer to the Table in the Catalog with given name.
 * Returns nullptr if not found. */
const Table* Database::find_table(const Varchar& name) const {
    auto it = tables_.find(name);
    if (it != tables_.end()) return &(it->second);
    return nullptr;
}

} // namespace minisql