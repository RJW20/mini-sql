#ifndef MINISQL_VALIDATOR_DEFAULTS_HPP
#define MINISQL_VALIDATOR_DEFAULTS_HPP

#include <cstddef>
#include <string>

#include "field/field.hpp"

namespace minisql::validator::defaults {

namespace primary {

inline const std::string NAME = "rowid";
inline const FieldType TYPE = FieldType::INT;
inline const std::size_t SIZE = sizeof(int);

} // namespace primary

inline const std::string ALL_COLUMNS = "*";

} // namespace minisql::validator::defaults

#endif // MINISQL_VALIDATOR_DEFAULTS_HPP