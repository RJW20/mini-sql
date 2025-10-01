#ifndef MINISQL_DATABASE_HANDLE_HPP
#define MINISQL_DATABASE_HANDLE_HPP

#include <memory>
#include <filesystem>
#include <utility>

#include "database.hpp"

namespace minisql {

class Engine;

/* Database Handle
 * Small RAII object that provides access to a Database. */
class DatabaseHandle {
public:
    DatabaseHandle(
        Engine& engine, std::shared_ptr<Database> db,
        const std::filesystem::path& path
    ) : engine_(engine), db_(std::move(db)), path_(std::move(path)) {}

    ~DatabaseHandle();

    Database* operator->() { return db_.get(); }
    Database& operator*() { return *db_; }

private:
    Engine& engine_;
    std::shared_ptr<Database> db_;
    std::filesystem::path path_;
};

} // namespace minisql

#endif // MINISQL_DATABASE_HANDLE_HPP