#ifndef MINISQL_ROW_HPP
#define MINISQL_ROW_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <minisql/field.hpp>

namespace minisql {

// Forward declare serialisation function.
class RowView;
class Row;
RowView serialise(const Row&);

// Schema forward declaration.
class Schema;

/* Row
 * A vector containing fields indexable by index or name. */
class Row {
public:
    Row(std::vector<Field> fields, std::shared_ptr<Schema> schema);

    const Field& operator[](std::size_t index) const;
    const Field& operator[](const std::string& name) const;

    auto begin() const { return fields_.begin(); }
    auto end() const { return fields_.end(); }
    std::size_t size() const { return fields_.size(); }

    friend RowView serialise(const Row&);

private:
    std::vector<Field> fields_;
    std::shared_ptr<Schema> schema_;
};

} // namespace minisql

#endif // MINISQL_ROW_HPP