#ifndef MINISQL_FRAME_HPP
#define MINISQL_FRAME_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
#include <list>

#include "frame_manager/disk_manager/page_id_t.hpp"

namespace minisql {

// In-memory object that can hold any page.
struct Frame {
    page_id_t pid {nullpid};
    std::vector<std::byte> data;
    bool dirty {false};
    std::uint16_t pin_count {0};

    std::list<std::size_t>::iterator lru_it {};
};

} // namespace minisql

#endif // MINISQL_FRAME_HPP