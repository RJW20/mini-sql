#ifndef MINISQL_ENGINE_HPP
#define MINISQL_ENGINE_HPP

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "database.hpp"
#include "engine/database_handle.hpp"
#include "minisql/row_set.hpp"

namespace minisql {

/* Custom hash for std::filesystem::path that ensures normalisation around .
 * and .. */
struct PathHash {
    std::size_t operator()(const std::filesystem::path& p) const {
        return std::hash<std::string>()(
            std::filesystem::weakly_canonical(p).string()
        );
    }
};

/* Custom equality for std::filesystem::path that ensures normalisation around
 * . and .. */
struct PathEqual {
    bool operator()(
        const std::filesystem::path& lhs, const std::filesystem::path& rhs
    ) const {
        return std::filesystem::weakly_canonical(lhs) ==
            std::filesystem::weakly_canonical(rhs);
    }
};

/* Engine
 * Manages access to Databases and handles execution of sql on them. */
class Engine {
public:
    DatabaseHandle open_database(const std::filesystem::path& path);
    void release_database(const std::filesystem::path& path);

    std::size_t exec(
        std::string_view sql, Database& db, bool master_enabled = false
    );
    RowSet query(std::string_view sql, Database& db);

private:
    std::unordered_map<
        std::filesystem::path, std::shared_ptr<Database>, PathHash, PathEqual
    > dbs_;
};

} // namespace minisql

#endif // MINISQL_ENGINE_HPP