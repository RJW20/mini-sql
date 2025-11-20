#ifndef MINISQL_DISK_MANAGER_HPP
#define MINISQL_DISK_MANAGER_HPP

#include <cstddef>
#include <fstream>

#include "frame_manager/disk_manager/page_id_t.hpp"

namespace minisql {

/* DiskManager
 * Reads pages from and writes pages to the file with given path.
 * page_id_t = 0 corresponds to the page starting from base_offset. */
class DiskManager {
public:
    DiskManager(
        std::fstream& file, std::streamoff base_offset,
        std::size_t page_size, page_id_t page_count
    );
    ~DiskManager() = default;

    DiskManager(const DiskManager&) = delete;
    DiskManager& operator=(const DiskManager&) = delete;

    void read(page_id_t pid, std::byte* dst);
    void write(page_id_t pid, const std::byte* src);
    void extend();
    
    std::size_t page_size() const noexcept { return page_size_; }
    page_id_t page_count() const noexcept { return page_count_; }

private:
    std::fstream& file_;
    const std::streamoff base_offset_;
    const std::size_t page_size_;
    page_id_t page_count_;

    std::streamoff page_offset(page_id_t pid) const {
        return base_offset_ + page_size_ * pid;
    }
};

} // namespace minisql

#endif // MINISQL_DISK_MANAGER_HPP