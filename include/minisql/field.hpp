#ifndef MINISQL_FIELD_HPP
#define MINISQL_FIELD_HPP

#include <variant>

#include <minisql/varchar.hpp>

namespace minisql {

// Variant holding any supported datatype.
using Field = std::variant<int, double, Varchar>;

} // namespace minisql

#endif // MINISQL_FIELD_HPP