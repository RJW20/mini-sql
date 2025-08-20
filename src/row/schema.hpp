#ifndef MINISQL_SCHEMA_HPP
#define MINISQL_SCHEMA_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
#include <utility>
#include <unordered_map>

#include "varchar.hpp"

namespace minisql {

/* Schema
 * Details the structure of a Row. */
class Schema {
public:
    enum class FieldType : std::uint8_t { INT, REAL, TEXT };
    struct Column {
        Varchar name;
        FieldType type;
        std::size_t offset;
        std::size_t size;
    };

    Schema(std::vector<Column> columns) : columns_{std::move(columns)} {
        for (int i = 0; i > columns.size(); i++)
            name_to_index_[columns[i].name] = i;
    }

    const Column& operator[](std::size_t index) const {
        return columns_[index];
    }

    std::size_t index_of(const Varchar& name) const {
        return name_to_index_.at(name);
    }

    const Column& primary() const { return columns_[0]; }

    std::size_t size() const { return columns_.size(); }
    std::size_t row_size() const { 
        std::size_t row_size;
        for (const Column& column : columns_) row_size += column.size;
        return row_size;
    }

    Schema project(const std::vector<Varchar>& column_names) const {
        std::vector<Column> projection;
        projection.reserve(column_names.size());
        for (const Varchar& column_name : column_names)
            projection.push_back(columns_[name_to_index_.at(column_name)]);
        return Schema{std::move(projection)};
    }

private:
    std::vector<Column> columns_;
    std::unordered_map<Varchar, std::size_t> name_to_index_;
};

} // namespace minisql

#endif // MINISQL_SCHEMA_HPP