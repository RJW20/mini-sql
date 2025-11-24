#ifndef MINISQL_ROW_SET_HPP
#define MINISQL_ROW_SET_HPP

#include <memory>

#include <minisql/row.hpp>
#include <minisql/row_iterator.hpp>

namespace minisql {

/* RowSet
 * Yields Rows from a SELECT statement. */
class RowSet {
public:
    RowSet();
    ~RowSet();

    RowSet(const RowSet&) = delete;
    RowSet& operator=(const RowSet&) = delete;

    RowSet(RowSet&&) noexcept;
    RowSet& operator=(RowSet&&) noexcept;

    // STL-style iteration
    RowIterator begin();
    RowIterator end();

    // Manual iteration
    bool next();
    Row current() const;

private:
    friend class Engine;
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace minisql

#endif // MINISQL_ROW_SET_HPP