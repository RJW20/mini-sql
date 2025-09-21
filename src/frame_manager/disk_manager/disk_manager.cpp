#include "frame_manager/disk_manager/disk_manager.hpp"

#include <cstddef>
#include <fstream>
#include <vector>

#include "frame_manager/disk_manager/page_id_t.hpp"
#include "exceptions.hpp"

namespace minisql {

DiskManager::DiskManager(
    std::fstream& file, std::streamoff base_offset,
    std::size_t page_size, page_id_t page_count
) : file_{file}, base_offset_{base_offset}, page_size_{page_size},
    page_count_{page_count} {
    file_.seekg(0, std::ios::end);
    if (file_.tellg() != page_offset(page_count))
        throw DiskException(page_offset(page_count), file_.tellg());
}

// Read the page corresponding to pid into dst.
void DiskManager::read(page_id_t pid, std::byte* dst) {
    const std::streamoff offset = page_offset(pid);
    if (pid >= page_count_)
        throw DiskException(offset, page_offset(page_count_));
    file_.seekg(offset);
    file_.read(reinterpret_cast<char*>(dst), page_size_);
}

// Write src into the page corresponding to pid.
void DiskManager::write(page_id_t pid, const std::byte* src) {
    const std::streamoff offset = page_offset(pid);
    if (pid >= page_count_)
        throw DiskException(offset, page_offset(page_count_));
    file_.seekp(offset);
    file_.write(reinterpret_cast<const char*>(src), page_size_);
}

// Extend file_ by one page.
void DiskManager::extend() {
    std::vector<char> zeros(page_size_);
    file_.seekp(0, std::ios::end);
    file_.write(zeros.data(), page_size_);
    page_count_++;
}

} // namespace minisql