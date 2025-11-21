#ifndef MINISQL_VALIDATOR_CONSTANTS_HPP
#define MINISQL_VALIDATOR_CONSTANTS_HPP

#include <cstddef>
#include <string>

#include "field/type.hpp"

namespace minisql::validator {
    
namespace defaults {

namespace primary {

inline const std::string NAME = "rowid";
inline const FieldType TYPE = FieldType::INT;
inline const std::size_t SIZE = sizeof(int);

} // namespace primary

inline const std::string ALL_COLUMNS = "*";

} // namespace defaults

namespace limits {

inline const std::size_t MAX_TABLE_WIDTH = 512;

} // namespace limits

} // namespace minisql::validator

#endif // MINISQL_VALIDATOR_CONSTANTS_HPP