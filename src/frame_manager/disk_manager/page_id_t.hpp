#ifndef MINISQL_PAGE_ID_T_HPP
#define MINISQL_PAGE_ID_T_HPP

#include <cstdint>
#include <limits>

namespace minisql {

// Logical page address on disk (0-based index in the file).
using page_id_t = std::uint32_t;

// page_id_t nullptr equivalent
constexpr page_id_t nullpid = std::numeric_limits<page_id_t>::max();

} // namespace minisql

#endif // MINISQL_PAGE_ID_T_HPP