#ifndef MINISQL_MASTER_TABLE_HPP
#define MINISQL_MASTER_TABLE_HPP

#include <cstddef>
#include <string>
#include <sstream>
#include <initializer_list>
#include <unordered_map>

#include "catalog/table.hpp"

namespace minisql::master_table {

inline const std::string NAME = "master";
inline const std::size_t MAX_TABLE_NAME_SIZE = 32;
inline const std::size_t MAX_SQL_SIZE = 256;

namespace columns {

struct Column { std::string name; std::string type; };

inline const Column TABLE_NAME = { "table_name", "TEXT(" + std::to_string(MAX_TABLE_NAME_SIZE) + ")" };
inline const Column SQL        = { "sql",        "TEXT(" + std::to_string(MAX_SQL_SIZE) + ")" };
inline const Column ROOT       = { "root",       "INT"       };
inline const Column NEXT_ROWID = { "next_rowid", "INT"       };

} // namespace columns

// Return a string containing sql for creating the master table.
inline std::string build_create_statement() {
    std::ostringstream sql;
    sql << "CREATE TABLE " << NAME << " ("
        << columns::TABLE_NAME.name << " " << columns::TABLE_NAME.type << ", "
        << columns::SQL.name        << " " << columns::SQL.type        << ", "
        << columns::ROOT.name       << " " << columns::ROOT.type       << ", "
        << columns::NEXT_ROWID.name << " " << columns::NEXT_ROWID.type << ", "
        << "PRIMARY KEY (" << columns::TABLE_NAME.name << "));";
    return sql.str();
}

/* Return a string containing sql for selecting the given columns from every
 * row in the master table. */
inline std::string build_select_statement(
    const std::initializer_list<columns::Column>& columns = {}
) {
    std::ostringstream sql;
    sql << "SELECT ";
    if (std::empty(columns)) sql << "*";
    else {
        bool first = true;
        for (const columns::Column& column : columns) {
            if (!first) sql << ", ";
            sql << column.name;
            first = false;
        }
    }
    sql << " FROM " << NAME << ";";
    return sql.str();
}

/* Return a string containing sql for inserting a row with the given table
 * information into the master_table. */
inline std::string build_insert_statement(
    const std::string& table_name, const std::string& create_sql,
    page_id_t root, rowid_t next_rowid
) {
    std::ostringstream sql;
    sql << "INSERT INTO " << master_table::NAME << " VALUES ("
        << "\"" << table_name << "\", \"" << create_sql << "\", " << root
        << ", " << next_rowid << ");";
    return sql.str();
}

/* Return a string containing sql for carrying out the given updates on the
 * row in the master table with given table_name.
 * Any new values that are strings should have enclosing quotes. */
inline std::string build_update_statement(
    const std::unordered_map<std::string, std::string>& updates,
    const std::string& table_name
) {
    std::ostringstream sql;
    sql << "UPDATE " << NAME << " SET ";
    bool first = true;
    for (const auto& [column, value] : updates) {
        if (!first) sql << ", ";
        sql << column << " = " << value;
        first = false;
    }
    sql << " WHERE " << columns::TABLE_NAME.name << " = \"" << table_name
        << "\";";
    return sql.str();
}


} // namespace minisql::master_table

#endif // MINISQL_MASTER_TABLE_HPP