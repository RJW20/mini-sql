#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include <cstddef>
#include <optional>
#include <sstream>
#include <string>
#include <variant>

#include <minisql/row.hpp>
#include <minisql/row_set.hpp>
#include <minisql/script_reader.hpp>

#include "connection.hpp"
#include "exceptions/exception.hpp"

/* Executor
 * Yields output from executing the given script on the given connection. */
class Executor {
public:
    Executor(minisql::ScriptReader& script, minisql::Connection& conn)
        : script_{script}, conn_{conn} {}

    std::optional<std::string> next() {

        if (row_set_.next()) return to_string(row_set_.current());
            
        auto statement = script_.next();
        if (!statement) return std::nullopt;
        try {
            if (statement->size() >= 6 &&
                !statement->compare(0, 6, "SELECT")) {
                row_set_ = conn_.query(*statement);
                return next();
            }
            else {
                std::size_t affected = conn_.exec(*statement);
                return std::to_string(affected) +
                    (affected == 1 ? " row" : " rows") + " affected";
            }
        }
        catch (const minisql::Exception& e) {
            return e.what();
        }
    }

private:
    minisql::ScriptReader& script_;
    minisql::Connection& conn_;
    minisql::RowSet row_set_ {nullptr};

    static std::string to_string(const minisql::Row& row) {
        std::ostringstream out;
        bool first {true};
        for (const auto& field : row) {
            if (!first) out << " | ";
            std::visit([&out](auto&& arg){ out << arg; }, field);
            first = false;
        }
        return out.str();
    }
};

#endif // EXECUTOR_HPP