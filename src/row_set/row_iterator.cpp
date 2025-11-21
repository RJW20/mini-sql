#include "minisql/row_iterator.hpp"

#include <utility>

#include "planner/iterators/iterator.hpp"

namespace minisql {

// Load the first Row into current_.
RowIterator::RowIterator(planner::Iterator*it) : it_{it} {
    advance();
}

RowIterator& RowIterator::operator++() {
    advance();
    return *this;
}

RowIterator RowIterator::operator++(int) {
    RowIterator tmp = std::move(*this);
    ++(*this);
    return tmp;
}

bool RowIterator::operator==(const RowIterator& other) const {
    return !valid_ && !other.valid_;
}

bool RowIterator::operator!=(const RowIterator& other) const {
    return !(*this == other);
}

// Load current_ with the next Row, or leave it empty if there is none.
void RowIterator::advance() { 
    if (it_ && it_->next()) {
        current_ = it_->current().deserialise();
        valid_ = true;
    }
    else {
        it_ = nullptr;
        current_ = {{}, nullptr};
        valid_ = false;
    }
}

} // namespace minisql