#include "row/row.hpp"

#include <cstddef>
#include <memory>
#include <vector>
#include <variant>
#include <utility>

#include "row/row_view.hpp"
#include "span.hpp"
#include "row/schema.hpp"
#include "field/field.hpp"
#include "byte_io.hpp"
#include "field/varchar.hpp"

namespace minisql {

RowView Row::serialise() const {
    auto owned = std::make_unique<std::vector<std::byte>>(schema_->row_size());
    span<std::byte> data = *owned;
    for (int i = 0; i < schema_->size(); i++) {
        const Schema::Column& column = (*schema_)[i];
        switch (column.type) {
            case FieldType::INT:
                byte_io::write<int>(
                    data, column.offset, std::get<int>(fields_[i])
                );
                break;
            case FieldType::REAL:
                byte_io::write<double>(
                    data, column.offset, std::get<double>(fields_[i])
                );
                break;
            case FieldType::TEXT:
                byte_io::write<Varchar>(
                    data, column.offset, std::get<Varchar>(fields_[i])
                );
                break;
        }
    }
    return RowView{data, schema_, std::move(owned)};
}

} // namespace minisql