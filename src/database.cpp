#include "database.hpp"

#include <cstddef>
#include <filesystem>
#include <vector>
#include <memory>

#include "frame_manager/disk_manager/page_id_t.hpp"
#include "headers.hpp"
#include "byte_io.hpp"
#include "frame_manager/frame_manager.hpp"
#include "catalog/catalog.hpp"

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
    catalog_ = std::make_unique<Catalog>(*fm_);
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

} // namespace minisql