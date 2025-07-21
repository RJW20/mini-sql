#ifndef MINISQL_CONNECTION_HPP
#define MINISQL_CONNECTION_HPP

#include <cstddef>
#include <filesystem>
#include <string_view>

namespace minisql {

/* */
class Connection {
public:
    explicit Connection(const std::filesystem::path& path);     // fetch or create db_

    std::size_t exec(std::string_view sql);     // DDL / DML
    Cursor query(std::string_view sql);         // SELECT
    Statement prepare(std::string_view sql);    // bind and step

private:
    Database* db_; // maybe a shared or weak ptr, need some kind of ref counting so can close it

};

} // namespace minisql

#endif // MINISQL_CONNECTION_HPP