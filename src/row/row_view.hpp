#ifndef MINISQL_ROW_VIEW_HPP
#define MINISQL_ROW_VIEW_HPP

#include <cstddef>
#include <string>
#include <memory>
#include <utility>
#include <vector>

#include "byte_io.hpp"
#include "field/type.hpp"
#include "minisql/field.hpp"
#include "minisql/varchar.hpp"
#include "row.hpp"
#include "row/schema.hpp"
#include "span.hpp"

namespace minisql {

/* Row View
 * Provides access to Fields in a Row without materialising the entire Row.
 * The span pointing to the underlying data either points to an external source
 * or into the vector held within owned_ if it is not nullptr. */
class RowView {
public:
    RowView(
        span<std::byte> data, std::shared_ptr<Schema> schema,
        std::unique_ptr<std::vector<std::byte>> owned = nullptr
    ) : data_{data}, schema_{std::move(schema)}, owned_{std::move(owned)} {}

    Field operator[](std::size_t index) const {
        const Schema::Column* column = (*schema_)[index];
        switch (column->type) {
            case FieldType::INT:
                return byte_io::view<int>(data_, column->offset);
            case FieldType::REAL:
                return byte_io::view<double>(data_, column->offset);
            case FieldType::TEXT:
                return byte_io::view<Varchar>(
                    data_, column->offset, column->size
                );
        }
        __builtin_unreachable();
    }

    Field operator[](const std::string& name) const {
        return (*this)[schema_->index_of(name)];
    }

    Field primary() const { return (*this)[schema_->primary().name]; }

    void set_field(std::size_t index, const Field& field) {
        const Schema::Column* column = (*schema_)[index];
        switch (column->type) {
            case FieldType::INT:
                byte_io::write<int>(
                    data_, column->offset, std::get<int>(field)
                );
                break;
            case FieldType::REAL:
                byte_io::write<double>(
                    data_, column->offset, std::get<double>(field)
                );
                break;
            case FieldType::TEXT:
                byte_io::write<Varchar>(
                    data_, column->offset, std::get<Varchar>(field)
                );
                break;
        }
    }

    void set_field(const std::string& name, const Field& field) {
        set_field(schema_->index_of(name), field);
    }

    Row deserialise() const {
        std::vector<Field> fields;
        fields.reserve(schema_->size());
        for (int i = 0; i < schema_->size(); i++) {
            const Schema::Column* column = (*schema_)[i];
            switch (column->type) {
                case FieldType::INT:
                    fields.push_back(
                        byte_io::copy<int>(data_, column->offset)
                    );
                    break;
                case FieldType::REAL:
                    fields.push_back(
                        byte_io::copy<double>(data_, column->offset)
                    );
                    break;
                case FieldType::TEXT:
                    fields.push_back(byte_io::copy<Varchar>(
                        data_, column->offset, column->size
                    ));
                    break;
            }
        }
        return Row{std::move(fields), schema_};
    }

    span<std::byte> data() const { return data_; }

private:
    span<std::byte> data_;
    std::shared_ptr<Schema> schema_;
    std::unique_ptr<std::vector<std::byte>> owned_;
};

} // namespace minisql

#endif // MINISQL_ROW_VIEW_HPP