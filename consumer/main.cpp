#include <filesystem>

#include <minisql/connection.hpp>

namespace fs = std::filesystem;

int main() {
    const fs::path db_path = "test.db";
    {
        minisql::Connection conn(db_path);
        conn.exec("CREATE TABLE t (int INT, real REAL, text TEXT(10));");
        conn.exec("DROP TABLE t;");
    }
    fs::remove(db_path);
    return 0;
}