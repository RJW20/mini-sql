#ifndef MINISQL_ROW_SET_HPP
#define MINISQL_ROW_SET_HPP

#include <memory>

#include <minisql/row.hpp>
#include <minisql/row_iterator.hpp>

namespace minisql {

namespace planner {

class Iterator;
using Plan = std::unique_ptr<Iterator>;

} // namespace planner

/* RowSet
 * Wrapper over a Plan. */
class RowSet {
public:
    ~RowSet();

    RowIterator begin();
    RowIterator end();

    bool next();
    Row current() const;

private:
    friend class Engine;
    explicit RowSet(planner::Plan plan);

    planner::Plan plan_;
};

} // namespace minisql

#endif // MINISQL_ROW_SET_HPP