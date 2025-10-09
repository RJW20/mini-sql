#include "validator/validator.hpp"

#include <variant>
#include <string>
#include <utility>
#include <algorithm>
#include <vector>

#include "row/schema.hpp"
#include "field/field.hpp"
#include "parser/ast.hpp"
#include "exceptions/query_exceptions.hpp"
#include "validator/query.hpp"
#include "catalog/catalog.hpp"
#include "engine/master_table.hpp"

namespace minisql::validator {

namespace {

/* Return a Field of the required type and size obtained from value.
 * Throws a FieldTypeException if not possible. */
Field validate(const parser::Value& value, const Schema::Column* column) {
    switch (column->type) {
        case FieldType::INT:
            if (!std::holds_alternative<double>(value))
                throw ColumnTypeException(column->name, "INT");
            return static_cast<int>(std::get<double>(value));
        case FieldType::REAL:
            if (!std::holds_alternative<double>(value))
                throw ColumnTypeException(column->name, "REAL");
            return std::get<double>(value);
        case FieldType::TEXT:
            if (!std::holds_alternative<std::string>(value))
                throw ColumnTypeException(column->name, "TEXT");
            return Varchar{std::get<std::string>(value).data(), column->size};
    }
    __builtin_unreachable();
}

/* Return a validated Condition from the given parser::Condition while:
 * - Verifying column's existence.
 * - Verifying value's type. */
Condition validate(const parser::Condition& condition, const Schema& schema) {

    const Schema::Column* column = schema[condition.column];
    if (!column) throw ColumnNotFoundException(condition.column);

    return {
        condition.column,
        Condition::Operator(static_cast<int>(condition.op)),
        validate(condition.value, column)
    };
}

/* Return a validated Modification from the given parser::Modification while:
 * - Verifying column's existence.
 * - Asserting column is not the primary column.
 * - Asserting that EQ is the only valid operator for TEXT columns.
 * - Verifying value's or the value column's type. */
Modification validate(
    const parser::Modification& modification, const Schema& schema
) {
    const Schema::Column* column = schema[modification.column];
    if (!column) throw ColumnNotFoundException(modification.column);

    if (column->name == schema.primary().name)
        throw ConstantColumnException(column->name);

    if (modification.op != parser::Modification::Operator::EQ &&
        column->type == FieldType::TEXT)
        throw ColumnTypeException(column->name, "TEXT");

    std::variant<Field, std::string> value;
    if (std::holds_alternative<parser::Value>(modification.value))
        value = validate(std::get<parser::Value>(modification.value), column);
    else {
        const Schema::Column* value_column = schema[
            std::get<std::string>(modification.value)
        ];
        if (!value_column)
            throw ColumnNotFoundException(
                std::get<std::string>(modification.value)
            );
        if (value_column->type != column->type)
            switch (column->type) {
                case FieldType::INT:
                    throw ColumnTypeException(column->name, "INT");
                case FieldType::REAL:
                    throw ColumnTypeException(column->name, "REAL");
                case FieldType::TEXT:
                    throw ColumnTypeException(column->name, "TEXT");
            }
            
        value = value_column->name;
    }

    return {
        modification.column,
        Modification::Operator(static_cast<int>(modification.op)),
        std::move(value)
    };
}

/* Return a validated CreateQuery from the given parser::CreateAST while:
 * - Verifying table doesn't exist.
 * - Asserting no column uses the reserved default primary name.
 * - Verifying the primary column exists if it is provided, and inserting a
 * default primary column otherwise. */
CreateQuery validate(const parser::CreateAST& ast, const Catalog& catalog) {

    const Table* table = catalog.find_table(ast.table);
    if (table) throw TableException(ast.table, true);
    CreateQuery query = {ast.table};

    for (const std::string& column : ast.columns)
        if (column == defaults::primary::NAME)
            throw ReservedColumnException(column);

    query.columns = ast.columns;
    query.types = ast.types;
    query.sizes = ast.sizes;
    
    if (ast.primary) {
        query.primary = *(ast.primary);
        auto it = std::find(
            query.columns.begin(), query.columns.end(), query.primary
        );
        if (it == query.columns.end())
            throw ColumnNotFoundException(query.primary);
    }
    else {
        query.columns.push_back(defaults::primary::NAME);
        query.types.push_back(defaults::primary::TYPE);
        query.sizes.push_back(defaults::primary::SIZE);
        query.primary = defaults::primary::NAME;
    }

    return query;
}

/* Return a validated SelectQuery from the given parser::SelectAST while:
 * - Verifying table's existence.
 * - Removing default primary column from the selection if all columns
 * requested.
 * - Verifying all column's existence.
 * - Validating all conditions. */
SelectQuery validate(const parser::SelectAST& ast, const Catalog& catalog) {

    const Table* table = catalog.find_table(ast.table);
    if (!table) throw TableException(ast.table, false);
    SelectQuery query = {ast.table};

    if (ast.columns.size() == 1 && ast.columns[0] == defaults::ALL_COLUMNS) {
        if (table->schema->primary().name == defaults::primary::NAME) {
            for (int i = 0; i < table->schema->size(); i++) {
            const Schema::Column* column = (*(table->schema))[i];
                if (column->name != defaults::primary::NAME)
                    query.columns.push_back(column->name);
            }
        }
        else query.columns.push_back(defaults::ALL_COLUMNS);
    }
    else {
        for (const std::string& column : ast.columns) {
            if (!(*(table->schema))[column])
                throw ColumnNotFoundException(column);
            query.columns.push_back(column);
        }
    }

    for (const parser::Condition& condition : ast.conditions)
        query.conditions.push_back(validate(condition, *(table->schema)));

    return query;
}

/* Return a validated InsertQuery from the given parser::InsertAST while:
 * - Verifying table's existence.
 * - Asserting table is not the master table if not enabled.
 * - Asserting all columns are present.
 * - Validating all values.
 * - Appending defualt values if the primary column is the default. */
InsertQuery validate(
    const parser::InsertAST& ast, Catalog& catalog, bool master_enabled
) {
    Table* table = catalog.find_table(ast.table);
    if (!table || !master_enabled && ast.table == master_table::NAME)
        throw TableException(ast.table, false);
    InsertQuery query = {ast.table};

    Schema* projected_schema;
    if (!(ast.columns.size() == 1 && ast.columns[0] == defaults::ALL_COLUMNS))
    {
        if (table->schema->primary().name != defaults::primary::NAME) {
            if (ast.columns.size() != table->schema->size())
                throw MissingColumnException();
        }
        else {
            if (ast.columns.size() != table->schema->size() - 1)
                throw MissingColumnException();
        }
        for (const std::string& column : ast.columns) {
            if (!(*(table->schema))[column])
                throw ColumnNotFoundException(column);
            query.columns.push_back(column);
        }
        bool use_rowid =
            table->schema->primary().name == defaults::primary::NAME;
        if (use_rowid) query.columns.push_back(defaults::primary::NAME);

        std::vector<Field> validated_row;

        validated_row.reserve(table->schema->size());
        for (const std::vector<parser::Value>& row : ast.values) {
            for (int i = 0; i < row.size(); i++) {
                const Schema::Column* column =
                    (*(table->schema))[query.columns[i]];
                validated_row.push_back(validate(row[i], column));
            }
            if (use_rowid)
                validated_row.push_back(static_cast<int>(table->next_rowid++));
            query.values.push_back(validated_row);
            validated_row.clear();
        }
    }
    else {
        query.columns.push_back(defaults::ALL_COLUMNS);
        bool use_rowid =
            table->schema->primary().name == defaults::primary::NAME;
        
        std::vector<Field> validated_row;
        validated_row.reserve(table->schema->size());
        for (const std::vector<parser::Value>& row : ast.values) {
            for (int i = 0; i < row.size(); i++) {
                const Schema::Column* column = (*(table->schema))[i];
                validated_row.push_back(validate(row[i], column));
            }
            if (use_rowid)
                validated_row.push_back(static_cast<int>(table->next_rowid++));
            query.values.push_back(validated_row);
            validated_row.clear();
        }
    }

    return query;
}

/* Return a validated UpdateQuery from the given parser::UpdateAST while:
 * - Verifying table's existence.
 * - Asserting table is not the master table if not enabled.
 * - Validating all modifications.
 * - Validating all conditions. */
UpdateQuery validate(
    const parser::UpdateAST& ast, const Catalog& catalog, bool master_enabled
) {
    const Table* table = catalog.find_table(ast.table);
    if (!table || !master_enabled && ast.table == master_table::NAME)
        throw TableException(ast.table, false);
    UpdateQuery query = {ast.table};

    for (const parser::Modification& modification : ast.modifications)
        query.modifications.push_back(
            validate(modification, *(table->schema))
        );

    for (const parser::Condition& condition : ast.conditions)
        query.conditions.push_back(validate(condition, *(table->schema)));

    return query;
}

/* Return a validated DeleteQuery from the given parser::DeleteAST while:
 * - Verifying table's existence.
 * - Asserting table is not the master table if not enabled.
 * - Validating all conditions. */
DeleteQuery validate(
    const parser::DeleteAST& ast, const Catalog& catalog, bool master_enabled
) {
    const Table* table = catalog.find_table(ast.table);
    if (!table || !master_enabled && ast.table == master_table::NAME)
        throw TableException(ast.table, false);
    DeleteQuery query = {ast.table};

    for (const parser::Condition& condition : ast.conditions)
        query.conditions.push_back(validate(condition, *(table->schema)));

    return query;
}

// Visitor struct for dispatching queries to correct validator.
struct Validator {
    Catalog& catalog;
    bool master_enabled;

    Query operator()(const parser::CreateAST& ast) const {
        return validate(ast, catalog);
    }
    Query operator()(const parser::SelectAST& ast) const {
        return validate(ast, catalog);
    }
    Query operator()(const parser::InsertAST& ast) const {
        return validate(ast, catalog, master_enabled);
    }
    Query operator()(const parser::UpdateAST& ast) const {
        return validate(ast, catalog, master_enabled);
    }
    Query operator()(const parser::DeleteAST& ast) const {
        return validate(ast, catalog, master_enabled);
    }
};

} // namespace

// Return a validated Query from the given parser::AST.
Query validate(const parser::AST& ast, Catalog& catalog, bool master_enabled) {
    return std::visit(Validator{catalog, master_enabled}, ast);
}

} // namespace minisql::validator