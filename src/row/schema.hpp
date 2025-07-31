#ifndef MINISQL_SCHEMA_HPP
#define MINISQL_SCHEMA_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
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

    Schema();

    const Column& operator[](std::size_t index) const {
        return columns_[index];
    }

    std::size_t index_of(Varchar name) const {
        return name_to_index_.at(name);
    }

    Schema project(const std::vector<Varchar>& column_names) const;

    std::size_t size() const { return columns_.size(); }
    std::size_t row_size() const { 
        std::size_t row_size;
        for (const Column& column : columns_) row_size += column.size;
        return row_size;
    }   

private:
    std::vector<Column> columns_;
    std::unordered_map<Varchar, std::size_t> name_to_index_;
};

} // namespace minisql

#endif // MINISQL_SCHEMA_HPP