#ifndef MINISQL_DATABASE_HPP
#define MINISQL_DATABASE_HPP

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory>

#include "catalog/catalog.hpp"
#include "frame_manager/disk_manager/page_id_t.hpp"
#include "frame_manager/frame_manager.hpp"

namespace minisql {

/* Database
 * Opens a FrameManager on the file with given path, and manages a catalog of
 * Tables within that file. */
class Database {
public:
    Database(const std::filesystem::path& path);
    ~Database();

    Catalog& catalog() { return *catalog_; };

    page_id_t master_root() const { return master_root_; }
    void set_master_root(page_id_t pid) { master_root_ = pid; }

private:
    std::fstream file_;
    page_id_t master_root_;
    std::unique_ptr<FrameManager> fm_;
    std::unique_ptr<Catalog> catalog_;

    // FrameManager arguments.
    static constexpr std::size_t PAGE_SIZE_ = 4096;
    static constexpr std::size_t CACHE_CAPACITY_ = 2000;
};

} // namespace minisql

#endif // MINISQL_DATABASE_HPP