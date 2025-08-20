#ifndef MINISQL_ROW_HPP
#define MINISQL_ROW_HPP

#include <cstddef>
#include <variant>
#include <vector>
#include <memory>
#include <utility>

#include "row/field.hpp"
#include "row/schema.hpp"
#include "varchar.hpp"

namespace minisql {

class RowView;

/* Row
 * A vector containing fields indexable by index or name. */
class Row {
public:
    Row(std::vector<Field> fields, std::shared_ptr<Schema> schema)
        : fields_{std::move(fields)}, schema_{std::move(schema)} {}

    const Field& operator[](std::size_t index) const { return fields_[index]; }

    const Field& operator[](Varchar name) const {
        return (*this)[schema_->index_of(name)];
    }

    RowView serialise() const;

private:
    std::vector<Field> fields_;
    std::shared_ptr<Schema> schema_;
};

} // namespace minisql

#endif // MINISQL_ROW_HPP