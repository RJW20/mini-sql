#ifndef MINISQL_FIELD_HPP
#define MINISQL_FIELD_HPP

#include <cstdint>
#include <variant>
#include <type_traits>

#include "varchar.hpp"

namespace minisql {

// Enum detailing supported datatypes.
enum class FieldType : std::uint8_t { INT, REAL, TEXT };

// Variant holding any supported datatype.
using Field = std::variant<int, double, Varchar>;

// Less than operator for 2 Fields of the same type.
template <typename T>
bool field_less_than(const Field& lhs, const Field& rhs) {
    return std::get<T>(lhs) < std::get<T>(rhs);
}

// Addition operator for 2 Fields of the same type.
template <typename T>
Field field_addition(const Field& lhs, const Field& rhs) {
    static_assert(
        std::is_arithmetic_v<T>, "Field type does not support addition."
    );
    return std::get<T>(lhs) + std::get<T>(rhs);
}

// Subtraction operator for 2 Fields of the same type.
template <typename T>
Field field_subtraction(const Field& lhs, const Field& rhs) {
    static_assert(
        std::is_arithmetic_v<T>, "Field type does not support subtraction."
    );
    return std::get<T>(lhs) - std::get<T>(rhs);
}

// Multiplication operator for 2 Fields of the same type.
template <typename T>
Field field_multiplication(const Field& lhs, const Field& rhs) {
    static_assert(
        std::is_arithmetic_v<T>, "Field type does not support mulitplication."
    );
    return std::get<T>(lhs) * std::get<T>(rhs);
}

// Division operator for 2 Fields of the same type.
template <typename T>
Field field_division(const Field& lhs, const Field& rhs) {
    static_assert(
        std::is_arithmetic_v<T>, "Field type does not support division."
    );
    return std::get<T>(lhs) / std::get<T>(rhs);
}

} // namespace minisql

#endif // MINISQL_FIELD_HPP