#include "database.hpp"

#include <cstddef>
#include <filesystem>
#include <vector>
#include <memory>
#include <string>
#include <utility>
#include <tuple>

#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"
#include "byte_io.hpp"
#include "exceptions.hpp"
#include "frame_manager/frame_manager.hpp"
#include "row/schema.hpp"
#include "catalog/table.hpp"
#include "bplus_tree/bplus_tree.hpp"

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
        flush_header(page_count, first_free_list_block);
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
        Magic magic = byte_io::view<Magic>(
            db_header, DatabaseHeader::MAGIC_OFFSET
        );
        if (magic != Magic::DATABASE)
            throw InvalidMagicException(magic);
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
    flush_header(fm_->page_count(), fm_->first_free_list_block());
}

// Construct a Table in the Catalog with given name.
void Database::add_table(
    const std::string& name, std::unique_ptr<Schema> schema, page_id_t root,
    rowid_t next_rowid
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
Table* Database::find_table(const std::string& name) {
    auto it = tables_.find(name);
    if (it != tables_.end()) return &(it->second);
    return nullptr;
}

/* Return a pointer to the const Table in the const Catalog with given name.
 * Returns nullptr if not found. */
const Table* Database::find_table(const std::string& name) const {
    auto it = tables_.find(name);
    if (it != tables_.end()) return &(it->second);
    return nullptr; 
}

// Write the database header to the start of file_.
void Database::flush_header(
    page_id_t page_count, page_id_t first_free_list_block
) {
    std::vector<std::byte> db_header{DatabaseHeader::SIZE};
    byte_io::write<Magic>(
        db_header, DatabaseHeader::MAGIC_OFFSET, Magic::DATABASE
    );
    byte_io::write<page_id_t>(
        db_header, DatabaseHeader::PAGE_COUNT_OFFSET, page_count
    );
    byte_io::write<page_id_t>(
        db_header, DatabaseHeader::FIRST_FREE_LIST_BLOCK_OFFSET,
        first_free_list_block
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

} // namespace minisql