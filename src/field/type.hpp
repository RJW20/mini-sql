#ifndef MINISQL_FIELD_TYPE_HPP
#define MINISQL_FIELD_TYPE_HPP

#include <cstdint>

namespace minisql {

// Enum detailing supported datatypes.
enum class FieldType : std::uint8_t { INT, REAL, TEXT };

} // namespace minisql

#endif // MINISQL_FIELD_TYPE_HPP