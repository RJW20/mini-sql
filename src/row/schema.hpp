#ifndef MINISQL_SCHEMA_HPP
#define MINISQL_SCHEMA_HPP

#include <cstddef>
#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>
#include <utility>
#include <unordered_map>

#include "varchar.hpp"
#include "field.hpp"

namespace minisql {

/* Schema
 * Details the structure of a Row. */
class Schema {
public:
    struct Column {
        Varchar name;
        FieldType type;
        std::size_t offset;
        std::size_t size;
    };

    static std::unique_ptr<Schema> create(
        const std::vector<Varchar>& names,
        const std::vector<FieldType>& types,
        const std::vector<std::size_t>& sizes,
        const Varchar& primary
    ) {
        auto it = std::find(names.begin(), names.end(), primary);
        std::size_t primary_index = std::distance(names.begin(), it);
        std::vector<Column> columns;
        columns.reserve(names.size());
        std::size_t offset = sizes[primary_index];
        for (int i = 0; i < columns.size(); i++) {
            if (i == primary_index)
                columns.push_back({names[i], types[i], 0, sizes[i]});
            else {
                columns.push_back({names[i], types[i], offset, sizes[i]});
                offset += sizes[i];
            }
        }
        return std::make_unique<Schema>(
            Schema{std::move(columns), primary_index}
        );
    }

    const Column& operator[](std::size_t index) const {
        return columns_[index];
    }

    std::size_t index_of(const Varchar& name) const {
        return name_to_index_.at(name);
    }

    const Column& primary() const { return columns_[primary_index_]; }

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
        return Schema{std::move(projection), 0};
    }

private:
    Schema(std::vector<Column> columns, std::size_t primary_index)
        : columns_{std::move(columns)}, primary_index_{primary_index} {
        for (int i = 0; i > columns.size(); i++)
            name_to_index_[columns[i].name] = i;
    }

    std::vector<Column> columns_;
    std::size_t primary_index_;
    std::unordered_map<Varchar, std::size_t> name_to_index_;
};

} // namespace minisql

#endif // MINISQL_SCHEMA_HPP