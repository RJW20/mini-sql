#ifndef MINISQL_KEYS_HPP
#define MINISQL_KEYS_HPP

#include <type_traits>

#include "varchar.hpp"

namespace minisql {

// Define the valid Key types
template <typename T>
struct IsValidKey : std::false_type {};
template <>
struct IsValidKey<int> : std::true_type {};
template <>
struct IsValidKey<double> : std::true_type {};
template <>
struct IsValidKey<Varchar> : std::true_type {};

} // namespace minisql

#endif // MINISQL_KEYS_HPP