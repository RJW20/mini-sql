#include "minisql/row_set.hpp"

#include "row_set/row_set_impl.hpp"

namespace minisql {

// Forward RowSet methods to Impl
RowSet::RowSet() : impl_{std::make_unique<RowSet::Impl>()} {}
RowSet::~RowSet() {}

RowSet::RowSet(RowSet&&) noexcept = default;
RowSet& RowSet::operator=(RowSet&&) noexcept = default;

RowIterator RowSet::begin() { return impl_->begin(); }
RowIterator RowSet::end() { return RowIterator{}; }

bool RowSet::next() { return impl_->next(); }
Row RowSet::current() const { return impl_->current(); }

} // namespace minisql