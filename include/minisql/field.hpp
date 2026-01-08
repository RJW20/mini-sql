#ifndef MINISQL_FIELD_HPP
#define MINISQL_FIELD_HPP

#include <variant>

#include <minisql/varchar.hpp>

namespace minisql {

namespace {

template <typename... Ts>
using FieldImpl = std::variant<Ts...>;

} // namespace

// Variant holding any supported datatype.
using Field = FieldImpl<
#define FIELD_TYPE(T) T,
#define FIELD_TYPE_LAST(T) T
#include "field_types.def"
#undef FIELD_TYPE
#undef FIELD_TYPE_LAST
>;

} // namespace minisql

#endif // MINISQL_FIELD_HPP