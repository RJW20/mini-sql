#ifndef MINISQL_FIELD_HPP
#define MINISQL_FIELD_HPP

#include <variant>

#include "varchar.hpp"

namespace minisql {

// Variant holding any supported datatype.
using Field = std::variant<int, double, Varchar>;

// Less than operator for 2 Fields of the same type.
template <typename T>
bool field_less_than(const Field& lhs, const Field& rhs) {
    return std::get<T>(lhs) < std::get<T>(rhs);
}

} // namespace minisql

#endif // MINISQL_FIELD_HPP