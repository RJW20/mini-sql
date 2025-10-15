#ifndef MINISQL_CONNECTION_HPP
#define MINISQL_CONNECTION_HPP

#include <cstddef>
#include <filesystem>
#include <string_view>

#include "row_set/row_set.hpp"
#include "engine/engine.hpp"
#include "engine/database_handle.hpp"

namespace minisql {

/* Connection.
 * Provides access to a Database via sql commands. */
class Connection {
public:
    explicit Connection(const std::filesystem::path& path)
        : dbh_{engine_.open_database(path)} {}

    // DDL / DML
    std::size_t exec(std::string_view sql) { return engine_.exec(sql, *dbh_); }

    // SELECT
    RowSet query(std::string_view sql) { return engine_.query(sql, *dbh_); }

private:
    inline static Engine engine_;
    DatabaseHandle dbh_;
};

} // namespace minisql

#endif // MINISQL_CONNECTION_HPP