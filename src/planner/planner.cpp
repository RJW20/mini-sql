#include "planner/planner.hpp"

#include <memory>
#include <vector>
#include <utility>
#include <optional>
#include <variant>

#include "planner/iterators/table_scan.hpp"
#include "cursor.hpp"
#include "row/schema.hpp"
#include "validator/query.hpp"
#include "planner/iterators/index_scan.hpp"
#include "planner/compiler.hpp"
#include "catalog/catalog.hpp"
#include "planner/iterators/create.hpp"
#include "catalog/table.hpp"
#include "planner/iterators/filter.hpp"
#include "planner/iterators/project.hpp"
#include "planner/iterators/values.hpp"
#include "planner/iterators/insert.hpp"
#include "planner/iterators/update.hpp"
#include "planner/iterators/erase.hpp"

namespace minisql::planner {

namespace {

/* Return a TableScan or IndexScan over the Rows held within the B+ Tree that
 * cursor corresponds to.
 * Iterates through conditions and applies them to the primary index directly
 * via an IndexScan or copies them into filter_conditions. */
std::unique_ptr<TableScan> make_scan(
    std::unique_ptr<Cursor> cursor, const Schema& schema, 
    const std::vector<validator::Condition>& conditions,
    std::vector<validator::Condition>& filter_conditions
) {
    if (conditions.empty())
        return std::make_unique<TableScan>(std::move(cursor), schema);

    std::unique_ptr<TableScan> scan;
    std::optional<validator::Condition> lower_bound;
    std::optional<validator::Condition> upper_bound;

    for (const validator::Condition& condition : conditions) {

        if (condition.column != schema.primary().name)
            filter_conditions.push_back(condition);

        if (condition.op == validator::Condition::Operator::EQ) {
            if (!scan) scan = std::make_unique<IndexScan>(
                std::move(cursor), schema, condition.value, true,
                condition.value, true
            );
            else filter_conditions.push_back(condition);
        }

        if (condition.op == validator::Condition::Operator::NEQ) {
            filter_conditions.push_back(condition);
            continue;
        }

        auto less_than = compile_less_than(schema[condition.column]->type);

        if (condition.op == validator::Condition::Operator::GT) {
            if (!lower_bound) {
                lower_bound = condition;
                continue;
            }
            if (less_than(condition.value, lower_bound->value)) continue;
            lower_bound = condition;
        }

        else if (condition.op == validator::Condition::Operator::GTE) {
            if (!lower_bound) {
                lower_bound = condition;
                continue;
            }
            if (less_than(condition.value, lower_bound->value) ||
                condition.value == lower_bound->value) continue;
            lower_bound = condition;
        }

        else if (condition.op == validator::Condition::Operator::LT) {
            if (!upper_bound) {
                upper_bound = condition;
                continue;
            }
            if (less_than(upper_bound->value, condition.value)) continue;
            upper_bound = condition;
        }

        else if (condition.op == validator::Condition::Operator::LTE) {
            if (!upper_bound) {
                upper_bound = condition;
                continue;
            }
            if (less_than(upper_bound->value, condition.value) ||
                upper_bound->value == condition.value) continue;
            upper_bound = condition;
        }
    }

    if (!scan) {
        if (!lower_bound) {
            if (!upper_bound) scan = std::make_unique<TableScan>(
                std::move(cursor), schema
            );
            else scan = std::make_unique<IndexScan>(
                std::move(cursor), schema, std::nullopt, false,
                std::move(upper_bound->value),
                upper_bound->op == validator::Condition::Operator::LTE
            );
        }
        else {
            if (!upper_bound) scan = std::make_unique<IndexScan>(
                std::move(cursor), schema, std::move(lower_bound->value),
                lower_bound->op == validator::Condition::Operator::GTE,
                std::nullopt, false
            );
            else scan = std::make_unique<IndexScan>(
                std::move(cursor), schema, std::move(lower_bound->value),
                lower_bound->op == validator::Condition::Operator::GTE,
                std::move(upper_bound->value),
                upper_bound->op == validator::Condition::Operator::LTE
            );
        }
    }
    else {
        if (lower_bound) {
            filter_conditions.push_back(std::move(*lower_bound));
            lower_bound.reset();
        }
        if (upper_bound) {
            filter_conditions.push_back(std::move(*upper_bound));
            upper_bound.reset();
        }
    }

    return scan;
}

// Return a Create iterator corresponding to a CreateQuery.
Plan plan(const validator::CreateQuery& query, Catalog& catalog) {
    return std::make_unique<Create>(
        catalog, query.table,
        Schema::create(query.columns, query.types, query.sizes, query.primary)
    );
}

/* Return an iterator tree corresponding to a SelectQuery.
 * Chains together a TableScan or IndexScan, and possibly a Filter and/or a
 * Project. */
Plan plan(const validator::SelectQuery& query, const Catalog& catalog) {

    const Table* table = catalog.find_table(query.table);
    auto cursor = std::make_unique<Cursor>(
        table->bp_tree.get(), *(table->schema)
    );

    std::vector<validator::Condition> filter_conditions;
    Plan plan = make_scan(
        std::move(cursor), *(table->schema), query.conditions,
        filter_conditions
    );

    if (!filter_conditions.empty()) plan = std::make_unique<Filter>(
        std::move(plan), compile(filter_conditions, *(table->schema))
    );

    if (query.columns[0] != "*") plan = std::make_unique<Project>(
        std::move(plan),
        std::make_shared<Schema>(table->schema->project(query.columns))
    );

    return plan;
}

/* Return an iterator tree corresponding to an InsertQuery.
 * Chains together a Values and an Insert. */
Plan plan(const validator::InsertQuery& query, const Catalog& catalog) {

    const Table* table = catalog.find_table(query.table);
    auto cursor = std::make_unique<Cursor>(
        table->bp_tree.get(), *(table->schema)
    );

    Plan plan = std::make_unique<Values>(
        query.values,
        std::make_shared<Schema>(
            query.columns[0] != "*" ?
            table->schema->project(query.columns) :
            *(table->schema)
        )
    );

    return std::make_unique<Insert>(std::move(plan), std::move(cursor));
}

/* Return an iterator tree corresponding to an UpdateQuery.
 * Chains together a TableScan or IndexScan, possibly a Filter, and an Update.
 */
Plan plan(const validator::UpdateQuery& query, const Catalog& catalog) {

    const Table* table = catalog.find_table(query.table);
    auto cursor = std::make_unique<Cursor>(
        table->bp_tree.get(), *(table->schema)
    );

    std::vector<validator::Condition> filter_conditions;
    Plan plan = make_scan(
        std::move(cursor), *(table->schema), query.conditions,
        filter_conditions
    );

    if (!filter_conditions.empty()) plan = std::make_unique<Filter>(
        std::move(plan), compile(filter_conditions, *(table->schema))
    );

    return std::make_unique<Update>(
        std::move(plan), compile(query.modifications, *(table->schema))
    );
}

/* Return an iterator tree corresponding to a DeleteQuery.
 * Chains together a TableScan or IndexScan, possibly a Filter, and an Erase.
 */
Plan plan(const validator::DeleteQuery& query, const Catalog& catalog) {

    const Table* table = catalog.find_table(query.table);
    auto cursor = std::make_unique<Cursor>(
        table->bp_tree.get(), *(table->schema)
    );
    Cursor* cursor_ptr = cursor.get();

    std::vector<validator::Condition> filter_conditions;
    Plan plan = make_scan(
        std::move(cursor), *(table->schema), query.conditions,
        filter_conditions
    );

    if (!filter_conditions.empty()) plan = std::make_unique<Filter>(
        std::move(plan), compile(filter_conditions, *(table->schema))
    );

    return std::make_unique<Erase>(std::move(plan), cursor_ptr);
}

// Visitor struct for dispatching validated queries to correct planner.
struct Planner {
    Catalog& catalog;

    Plan operator()(const validator::CreateQuery& query) const {
        return plan(query, catalog);
    }
    Plan operator()(const validator::SelectQuery& query) const {
        return plan(query, catalog);
    }
    Plan operator()(const validator::InsertQuery& query) const {
        return plan(query, catalog);
    }
    Plan operator()(const validator::UpdateQuery& query) const {
        return plan(query, catalog);
    }
    Plan operator()(const validator::DeleteQuery& query) const {
        return plan(query, catalog);
    }
};

} // namespace

// Return a plan according to the given Query.
Plan plan(const validator::Query& query, Catalog& catalog) {
    return std::visit(Planner{catalog}, query);
}

} // namespace minisql::planner