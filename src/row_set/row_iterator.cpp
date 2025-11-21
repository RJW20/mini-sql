#include "minisql/row_iterator.hpp"

#include "planner/iterators/iterator.hpp"

namespace minisql {

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