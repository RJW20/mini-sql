#ifndef MINISQL_ROW_ITERATOR_HPP
#define MINISQL_ROW_ITERATOR_HPP

#include <iterator>
#include <utility>

#include "planner/iterators/iterator.hpp"
#include "row/row.hpp"

namespace minisql {

/* Row Iterator
 * STL-style iterator that materialises Rows from a planner::Iterator. */
class RowIterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Row;
    using difference_type = std::ptrdiff_t;
    using pointer = Row*;
    using reference = Row&;

    explicit RowIterator(planner::Iterator* it = nullptr) : it_{it} {
        advance();
    }

    reference operator*() { return current_; }
    pointer operator->() { return &current_; }

    RowIterator& operator++() {
        advance();
        return *this;
    }

    RowIterator operator++(int) {
        RowIterator tmp = std::move(*this);
        ++(*this);
        return tmp;
    }

    bool operator==(const RowIterator& other) const {
        return !valid_ && !other.valid_;
    }

    bool operator!=(const RowIterator& other) const {
        return !(*this == other);
    }

private:
    planner::Iterator* it_;
    Row current_ {{}, nullptr};
    bool valid_ {false};

    void advance() {
        if (it_ && it_->next()) {
            current_ = it_->current().deserialise();
            valid_ = true;
        }
        else {
            if (it_) it_->close();
            it_ = nullptr;
            current_ = {{}, nullptr};
            valid_ = false;
        }
    }
};

} // namespace minisql

#endif // MINISQL_ROW_ITERATOR_HPP