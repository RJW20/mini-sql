#ifndef MINISQL_EXCEPTION_HPP
#define MINISQL_EXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace minisql {

/* Base class for all minisql exceptions.
 * All custom exceptions inherit from this, so they can be caught uniformly. */
class Exception : public std::runtime_error {
public:
    explicit Exception(const std::string& msg) : std::runtime_error(msg) {}
};

} // namespace minisql

#endif // MINISQL_EXCEPTION_hpp