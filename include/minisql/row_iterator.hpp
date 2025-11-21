#ifndef MINISQL_ROW_ITERATOR_HPP
#define MINISQL_ROW_ITERATOR_HPP

#include <iterator>

#include <minisql/row.hpp>

namespace minisql {

namespace planner { class Iterator; }

/* Row Iterator
 * STL-style iterator that materialises Rows from a planner::Iterator. */
class RowIterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Row;
    using difference_type = std::ptrdiff_t;
    using pointer = Row*;
    using reference = Row&;

    explicit RowIterator(planner::Iterator* it = nullptr);

    reference operator*() { return current_; }
    pointer operator->() { return &current_; }

    RowIterator& operator++();
    RowIterator operator++(int);

    bool operator==(const RowIterator& other) const;
    bool operator!=(const RowIterator& other) const;

private:
    planner::Iterator* it_;
    Row current_ {{}, nullptr};
    bool valid_ {false};

    void advance();
};

} // namespace minisql

#endif // MINISQL_ROW_ITERATOR_HPP