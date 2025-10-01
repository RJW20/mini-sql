#include "planner/compiler.hpp"

#include <functional>
#include <utility>
#include <variant>
#include <string>
#include <vector>

#include "field/field.hpp"
#include "field/varchar.hpp"
#include "exceptions.hpp"
#include "validator/query.hpp"
#include "row/schema.hpp"
#include "row/row_view.hpp"

namespace minisql::planner {

/* Return the template instance of field_less_than corresponding to the given
 * FieldType. */ 
std::function<bool(const Field&, const Field&)> compile_less_than(
    FieldType type
) {
    switch (type) {
        case FieldType::INT: return field_less_than<int>;
        case FieldType::REAL: return field_less_than<double>;
        case FieldType::TEXT: return field_less_than<Varchar>;
    };
    __builtin_unreachable();
}

namespace {

/* Return the template instance of field_addition corresponding to the given
 * FieldType. */
std::function<Field(const Field&, const Field&)> compile_addition(
    FieldType type
) {
    switch (type) {
        case FieldType::INT: return field_addition<int>;
        case FieldType::REAL: return field_addition<double>;
        case FieldType::TEXT: throw FieldTypeException("INT/REAL", "TEXT");
    };
    __builtin_unreachable();
}

/* Return the template instance of field_subtraction corresponding to the given
 * FieldType. */
std::function<Field(const Field&, const Field&)> compile_subtraction(
    FieldType type
) {
    switch (type) {
        case FieldType::INT: return field_subtraction<int>;
        case FieldType::REAL: return field_subtraction<double>;
        case FieldType::TEXT: throw FieldTypeException("INT/REAL", "TEXT");
    };
    __builtin_unreachable();
}

/* Return the template instance of field_multiplication corresponding to the
 * given FieldType. */
std::function<Field(const Field&, const Field&)> compile_multiplication(
    FieldType type
) {
    switch (type) {
        case FieldType::INT: return field_multiplication<int>;
        case FieldType::REAL: return field_multiplication<double>;
        case FieldType::TEXT: throw FieldTypeException("INT/REAL", "TEXT");
    };
    __builtin_unreachable();
}

/* Return the template instance of field_division corresponding to the given
 * FieldType. */
std::function<Field(const Field&, const Field&)> compile_division(
    FieldType type
) {
    switch (type) {
        case FieldType::INT: return field_division<int>;
        case FieldType::REAL: return field_division<double>;
        case FieldType::TEXT: throw FieldTypeException("INT/REAL", "TEXT");
    };
    __builtin_unreachable();
}

Predicate compile(const validator::Condition& condition, const Schema& schema)
{
    auto less_than = compile_less_than(
        schema[schema.index_of(condition.column)].type
    );

    switch (condition.op) {
        case validator::Condition::Operator::EQ:
            return [col = condition.column, rhs = condition.value]
                    (const RowView& rv) { return rv[col] == rhs; };
        case validator::Condition::Operator::NEQ:
            return [col = condition.column, rhs = condition.value]
                    (const RowView& rv) { return rv[col] != rhs; };
        case validator::Condition::Operator::GT:
            return [col = condition.column, lhs = condition.value,
                    lt = std::move(less_than)]
                    (const RowView& rv) { return lt(lhs, rv[col]); };
        case validator::Condition::Operator::GTE:
            return [col = condition.column, lhs = condition.value,
                    lt = std::move(less_than)]
                    (const RowView& rv) {
                return lt(lhs, rv[col]) || lhs == rv[col];
            };
        case validator::Condition::Operator::LT:
            return [col = condition.column, rhs = condition.value,
                    lt = std::move(less_than)]
                    (const RowView& rv) { return lt(rv[col], rhs); };
        case validator::Condition::Operator::LTE:
            return [col = condition.column, rhs = condition.value,
                    lt = std::move(less_than)]
                    (const RowView& rv) {
                return lt(rv[col], rhs) || rv[col] == rhs;
            };
    }

    __builtin_unreachable();
}

Modifier compile(
    const validator::Modification& modification, const Schema& schema
) {
    switch (modification.op) {
        case validator::Modification::Operator::EQ:
            if (std::holds_alternative<Field>(modification.value))
                return [col = modification.column,
                        value = std::get<Field>(modification.value)]
                        (RowView& rv) { rv.set_field(col, value); };
            else
                return [col1 = modification.column,
                        col2 = std::get<std::string>(modification.value)]
                        (RowView& rv) { rv.set_field(col1, rv[col2]); };
        case validator::Modification::Operator::ADD: {
            auto add = compile_addition(
                schema[schema.index_of(modification.column)].type
            );
            if (std::holds_alternative<Field>(modification.value))
                return [col = modification.column,
                        value = std::get<Field>(modification.value),
                        add = std::move(add)]
                        (RowView& rv) {
                    rv.set_field(col, add(rv[col], value));
                };
            else
                return [col1 = modification.column,
                        col2 = std::get<std::string>(modification.value),
                        add = std::move(add)]
                        (RowView& rv) {
                    rv.set_field(col1, add(rv[col1], rv[col2]));
                };
            }
        case validator::Modification::Operator::SUB: {
            auto sub = compile_subtraction(
                schema[schema.index_of(modification.column)].type
            );
            if (std::holds_alternative<Field>(modification.value))
                return [col = modification.column,
                        value = std::get<Field>(modification.value),
                        sub = std::move(sub)]
                        (RowView& rv) {
                    rv.set_field(col, sub(rv[col], value));
                };
            else
                return [col1 = modification.column,
                        col2 = std::get<std::string>(modification.value),
                        sub = std::move(sub)]
                        (RowView& rv) {
                    rv.set_field(col1, sub(rv[col1], rv[col2]));
                };
            }
        case validator::Modification::Operator::MUL: {
            auto mul = compile_multiplication(
                schema[schema.index_of(modification.column)].type
            );
            if (std::holds_alternative<Field>(modification.value))
                return [col = modification.column,
                        value = std::get<Field>(modification.value),
                        mul = std::move(mul)]
                        (RowView& rv) {
                    rv.set_field(col, mul(rv[col], value));
                };
            else
                return [col1 = modification.column,
                        col2 = std::get<std::string>(modification.value),
                        mul = std::move(mul)]
                        (RowView& rv) {
                    rv.set_field(col1, mul(rv[col1], rv[col2]));
                };
            }
        case validator::Modification::Operator::DIV: {
            auto div = compile_division(
                schema[schema.index_of(modification.column)].type
            );
            if (std::holds_alternative<Field>(modification.value))
                return [col = modification.column,
                        value = std::get<Field>(modification.value),
                        div = std::move(div)]
                        (RowView& rv) {
                    rv.set_field(col, div(rv[col], value));
                };
            else
                return [col1 = modification.column,
                        col2 = std::get<std::string>(modification.value),
                        div = std::move(div)]
                        (RowView& rv) {
                    rv.set_field(col1, div(rv[col1], rv[col2]));
                };
            }
    }

    __builtin_unreachable();
}

} // namespace

// Compile conditions into one function that returns true when all are met.
Predicate compile(
    const std::vector<validator::Condition>& conditions, const Schema& schema
) {
    Predicate predicate = [](const RowView&){ return true; };
    for (const validator::Condition& condition : conditions)
        predicate = [previous = std::move(predicate),
                     current = compile(condition, schema)]
                     (const RowView& rv) {
            return previous(rv) && current(rv);
        };
    return std::move(predicate);
}

// Compile modifications into one function that modifies a RowView.
Modifier compile(
    const std::vector<validator::Modification>& modifications,
    const Schema& schema
) {
    Modifier modifier = [](RowView&){ return; };
    for (const validator::Modification& modification : modifications)
        modifier = [previous = std::move(modifier),
                    current = compile(modification, schema)]
                    (RowView& rv) { previous(rv); current(rv); };
    return std::move(modifier);
}

} // namespace minisql::planner