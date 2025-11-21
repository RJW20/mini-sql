#ifndef MINISQL_CONNECTION_HPP
#define MINISQL_CONNECTION_HPP

#include <cstddef>
#include <filesystem>
#include <string_view>

#include <minisql/row_set.hpp>

namespace minisql {

/* Connection.
 * Provides access to a Database via sql commands. */
class Connection {
public:
    explicit Connection(const std::filesystem::path& path);
    ~Connection();

    // DDL / DML
    std::size_t exec(std::string_view sql);

    // SELECT
    RowSet query(std::string_view sql);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace minisql

#endif // MINISQL_CONNECTION_HPP