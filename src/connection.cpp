#include "minisql/connection.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string_view>

#include "engine/database_handle.hpp"
#include "engine/engine.hpp"
#include "minisql/row_set.hpp"

namespace minisql {

// Implementation of Connection methods.
class Connection::Impl {
public:
    explicit Impl(const std::filesystem::path& path)
        : dbh_{engine_.open_database(path)} {}

    std::size_t exec(std::string_view sql) { return engine_.exec(sql, *dbh_); }

    RowSet query(std::string_view sql) { return engine_.query(sql, *dbh_); }

private:
    inline static Engine engine_;
    DatabaseHandle dbh_;
};

// Forward Connection methods to Impl
Connection::Connection(const std::filesystem::path& path)
    : impl_{std::make_unique<Impl>(path)} {}
Connection::~Connection() {}
std::size_t Connection::exec(std::string_view sql) { return impl_->exec(sql); }
RowSet Connection::query(std::string_view sql) { return impl_->query(sql); }

} // namespace minisql