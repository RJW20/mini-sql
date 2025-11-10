#ifndef MINISQL_DATABASE_HPP
#define MINISQL_DATABASE_HPP

#include <cstddef>
#include <filesystem>
#include <string>
#include <memory>
#include <fstream>

#include "catalog/catalog.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "row/schema.hpp"
#include "frame_manager/frame_manager.hpp"

namespace minisql {

/* Database
 * Opens a FrameManager on the file with given path, and manages a catalog of
 * Tables within that file. */
class Database : public Catalog {
public:
    Database(const std::filesystem::path& path);
    ~Database();

    page_id_t master_root() const { return master_root_; }
    void set_master_root(page_id_t pid) { master_root_ = pid; }

    void add_table(
        const std::string& name, std::unique_ptr<Schema> schema,
        page_id_t root = nullpid, rowid_t next_rowid = 0
    ) override;

private:
    std::fstream file_;
    page_id_t master_root_;
    std::unique_ptr<FrameManager> fm_;

    void flush_header(page_id_t page_count, page_id_t first_free_list_block);

    // FrameManager arguments.
    static constexpr std::size_t PAGE_SIZE_ = 4096;
    static constexpr std::size_t CACHE_CAPACITY_ = 2000;
};

} // namespace minisql

#endif // MINISQL_DATABASE_HPP