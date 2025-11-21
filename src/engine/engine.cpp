#include "engine/engine.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "database.hpp"
#include "engine/database_handle.hpp"
#include "engine/master_table.hpp"
#include "minisql/row.hpp"
#include "minisql/row_set.hpp"
#include "minisql/varchar.hpp"
#include "parser/ast.hpp"
#include "parser/parser.hpp"
#include "planner/planner.hpp"
#include "row/schema.hpp"
#include "validator/query.hpp"
#include "validator/validator.hpp"

namespace minisql {

/* Return a DatabaseHandle providing access to the database with given path.
 * If the database is already open then a new DatabaseHandle is created and
 * returned.
 * Otherwise the database is opened and stored, the master table and its
 * contents is added to the database's catalog and then a new DatabaseHandle
 * is created and returned. */
DatabaseHandle Engine::open_database(const std::filesystem::path& path) {
    auto it = dbs_.find(path);
    if (it != dbs_.end()) return DatabaseHandle{*this, it->second, path};
    auto db = std::make_shared<Database>(path);
    auto create_ast = std::get<parser::CreateAST>(
        parser::parse(master_table::build_create_statement())
    );
    db->add_table(
        master_table::NAME,
        Schema::create(
            create_ast.columns, create_ast.types, create_ast.sizes,
            *(create_ast.primary)
        ),
        db->master_root()
    );
    RowSet tables = query(master_table::build_select_statement(), *db);
    for (Row& table_info : tables) {
        parser::AST ast = parser::parse(std::get<Varchar>(
            table_info[master_table::columns::SQL.name]
        ).data());
        auto c_query = std::get<validator::CreateQuery>(
            validator::validate(ast, *db, false)
        );
        db->add_table(
            std::get<Varchar>(
                table_info[master_table::columns::TABLE_NAME.name]
            ).data(),
            Schema::create(
                c_query.columns, c_query.types, c_query.sizes, c_query.primary
            ),
            std::get<int>(table_info[master_table::columns::ROOT.name]),
            std::get<int>(table_info[master_table::columns::NEXT_ROWID.name])
        );
    }
    dbs_[path] = db;
    return DatabaseHandle(*this, std::move(db), path);
}

/* Close the database with given path if there are no Connections using it.
 * Updates its master table and master_root before closing. */
void Engine::release_database(const std::filesystem::path& path) {
    auto it = dbs_.find(path);
    if (it == dbs_.end()) return;
    auto db = it->second;
    if (db.use_count() > 2) return;
    RowSet tables = query(
        master_table::build_select_statement(
            {master_table::columns::TABLE_NAME}
        ),
        *db
    );
    for (Row& table_info : tables) {
        const std::string& table_name = std::get<Varchar>(
            table_info[master_table::columns::TABLE_NAME.name]
        ).data();
        const Table* table = db->find_table(table_name);
        exec(
            master_table::build_update_statement(
                {
                    {
                        master_table::columns::ROOT.name,
                        std::to_string(table->bp_tree->root())
                    },
                    {
                        master_table::columns::NEXT_ROWID.name,
                        std::to_string(table->next_rowid)
                    }
                }, table_name
            ), *db, true
        );
    }
    db->set_master_root(db->find_table(master_table::NAME)->bp_tree->root());
    dbs_.erase(it);
}

// Execute the given sql on the given database.
std::size_t Engine::exec(
    std::string_view sql, Database& db, bool master_enabled
) {
    parser::AST ast = parser::parse(sql);
    validator::Query query = validator::validate(ast, db, master_enabled);
    planner::Plan plan = planner::plan(query, db);
    while (plan->next());
    if (std::holds_alternative<validator::CreateQuery>(query)) {
        std::string table_name = std::get<validator::CreateQuery>(query).table;
        const Table* table = db.find_table(table_name);
        exec(
            master_table::build_insert_statement(
                table_name, sql.data(), table->bp_tree->root(),
                table->next_rowid
            ), db, true
        );
    }
    else if (std::holds_alternative<validator::DropQuery>(query)) {
        std::string table_name = std::get<validator::DropQuery>(query).table;
        exec(master_table::build_delete_statement(table_name), db, true);
    }
    return plan->count();
}

/* Return a RowSet containing all rows output when executing the given sql on
 * the given database. */
RowSet Engine::query(std::string_view sql, Database& db) {
    parser::AST ast = parser::parse(sql);
    validator::Query query = validator::validate(ast, db, false);
    planner::Plan plan = planner::plan(query, db);
    return RowSet{std::move(plan)};
}

} // namespace minisql