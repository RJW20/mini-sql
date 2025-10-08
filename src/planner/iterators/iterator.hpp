#ifndef MINISQL_PLANNER_ITERATOR_HPP
#define MINISQL_PLANNER_ITERATOR_HPP

#include <cstddef>

#include "row/row_view.hpp"

namespace minisql::planner {

// Base class for a plan iterator.
class Iterator {
public:
    virtual ~Iterator() = default;
    virtual bool next() = 0;
    virtual RowView current() = 0;
    std::size_t count() const { return count_; }
    
protected:
    std::size_t count_ {0};
};

} // namespace minisql::planner

#endif // MINISQL_PLANNER_ITERATOR_HPP