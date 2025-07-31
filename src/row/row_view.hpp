#ifndef MINISQL_ROW_VIEW_HPP
#define MINISQL_ROW_VIEW_HPP

#include <cstddef>
#include <memory>
#include <vector>
#include <utility>

#include "span.hpp"
#include "row/schema.hpp"
#include "row.hpp"
#include "byte_io.hpp"
#include "varchar.hpp"

namespace minisql {

/* Row View
 * Provides access to fields in a Row without materialising the entire Row.
 * The span pointing to the underlying data either points to an external source
 * or into the vector held within owned_ if it is not nullptr. */
class RowView {
public:
    RowView(
        span<std::byte> data, std::shared_ptr<Schema> schema,
        std::unique_ptr<std::vector<std::byte>> owned = nullptr
    ) : data_{data}, schema_{std::move(schema)}, owned_{std::move(owned)} {}

    Row::Field operator[](std::size_t index) const {
        const Schema::Column& column = (*schema_)[index];
        switch (column.type) {
            case Schema::FieldType::INT:
                return ByteIO::view<int>(data_, column.offset);
            case Schema::FieldType::REAL:
                return ByteIO::view<double>(data_, column.offset);
            case Schema::FieldType::TEXT:
                return ByteIO::view<Varchar>(
                    data_, column.offset, column.size
                );
        }
    }

    Row::Field operator[](Varchar name) const {
        return (*this)[schema_->index_of(name)];
    }

    Row deserialise() const {
        std::vector<Row::Field> fields;
        fields.reserve(schema_->size());
        for (int i = 0; i < schema_->size(); i++) {
            const Schema::Column& column = (*schema_)[i];
            switch (column.type) {
                case Schema::FieldType::INT:
                    fields.push_back(ByteIO::copy<int>(data_, column.offset));
                    break;
                case Schema::FieldType::REAL:
                    fields.push_back(
                        ByteIO::copy<double>(data_, column.offset)
                    );
                    break;
                case Schema::FieldType::TEXT:
                    fields.push_back(ByteIO::copy<Varchar>(
                        data_, column.offset, column.size
                    ));
                    break;
            }
        }
        return Row{std::move(fields), schema_};
    }

private:
    span<std::byte> data_;
    std::shared_ptr<Schema> schema_;
    std::unique_ptr<std::vector<std::byte>> owned_;
};

} // namespace minisql

#endif // MINISQL_ROW_VIEW_HPP