#include "minisql/row.hpp"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>
#include <variant>

#include "byte_io.hpp"
#include "field/type.hpp"
#include "minisql/field.hpp"
#include "minisql/varchar.hpp"
#include "row/row_view.hpp"
#include "row/schema.hpp"
#include "span.hpp"

namespace minisql {

Row::Row(std::vector<Field> fields, std::shared_ptr<Schema> schema)
    : fields_{std::move(fields)}, schema_{std::move(schema)} {}

const Field& Row::operator[](std::size_t index) const {
    return fields_[index];
}

const Field& Row::operator[](const std::string& name) const {
    return (*this)[schema_->index_of(name)];
}

RowView serialise(const Row& row) {
    auto owned = std::make_unique<std::vector<std::byte>>(
        row.schema_->row_size()
    );
    span<std::byte> data = *owned;
    for (int i = 0; i < row.schema_->size(); i++) {
        const Schema::Column* column = (*row.schema_)[i];
        switch (column->type) {
            case FieldType::INT:
                byte_io::write<int>(
                    data, column->offset, std::get<int>(row.fields_[i])
                );
                break;
            case FieldType::REAL:
                byte_io::write<double>(
                    data, column->offset, std::get<double>(row.fields_[i])
                );
                break;
            case FieldType::TEXT:
                byte_io::write<Varchar>(
                    data, column->offset, std::get<Varchar>(row.fields_[i])
                );
                break;
        }
    }
    return RowView{data, row.schema_, std::move(owned)};
}

} // namespace minisql