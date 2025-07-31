#include "row/row.hpp"

#include <cstddef>
#include <memory>
#include <vector>
#include <variant>
#include <utility>

#include "row/row_view.hpp"
#include "span.hpp"
#include "byte_io.hpp"

namespace minisql {

RowView Row::serialise() const {
    auto owned = std::make_unique<std::vector<std::byte>>(schema_->row_size());
    span<std::byte> data = *owned;
    for (int i = 0; i < schema_->size(); i++) {
        const Schema::Column& column = (*schema_)[i];
        switch (column.type) {
            case Schema::FieldType::INT:
                ByteIO::write<int>(
                    data, column.offset, std::get<int>(fields_[i])
                );
                break;
            case Schema::FieldType::REAL:
                ByteIO::write<double>(
                    data, column.offset, std::get<double>(fields_[i])
                );
                break;
            case Schema::FieldType::TEXT:
                ByteIO::write<Varchar>(
                    data, column.offset, std::get<Varchar>(fields_[i])
                );
                break;
        }
    }
    return RowView{data, schema_, std::move(owned)};
}

} // namespace minisql