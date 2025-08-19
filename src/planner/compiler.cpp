#include "planner/compiler.hpp"

#include <functional>

#include "row/field.hpp"
#include "row/schema.hpp"
#include "varchar.hpp"
#include "exceptions.hpp"

namespace minisql::planner {

/* Return the template instance of field_less_than corresponding to the given
 * FieldType. */ 
std::function<bool(const Field&, const Field&)> compile_less_than(
    Schema::FieldType type
) {
    switch (type) {
        case Schema::FieldType::INT: return field_less_than<int>;
        case Schema::FieldType::REAL: return field_less_than<double>;
        case Schema::FieldType::TEXT: return field_less_than<Varchar>;
    };
    __builtin_unreachable();
}

namespace {

/* Return the template instance of field_addition corresponding to the given
 * FieldType. */
std::function<Field(const Field&, const Field&)> compile_addition(
    Schema::FieldType type
) {
    switch (type) {
        case Schema::FieldType::INT: return field_addition<int>;
        case Schema::FieldType::REAL: return field_addition<double>;
        case Schema::FieldType::TEXT:
            throw FieldTypeException("INT/REAL", "TEXT");
    };
    __builtin_unreachable();
}

/* Return the template instance of field_subtraction corresponding to the given
 * FieldType. */
std::function<Field(const Field&, const Field&)> compile_subtraction(
    Schema::FieldType type
) {
    switch (type) {
        case Schema::FieldType::INT: return field_subtraction<int>;
        case Schema::FieldType::REAL: return field_subtraction<double>;
        case Schema::FieldType::TEXT:
            throw FieldTypeException("INT/REAL", "TEXT");
    };
    __builtin_unreachable();
}

/* Return the template instance of field_multiplication corresponding to the
 * given FieldType. */
std::function<Field(const Field&, const Field&)> compile_multiplication(
    Schema::FieldType type
) {
    switch (type) {
        case Schema::FieldType::INT: return field_multiplication<int>;
        case Schema::FieldType::REAL: return field_multiplication<double>;
        case Schema::FieldType::TEXT:
            throw FieldTypeException("INT/REAL", "TEXT");
    };
    __builtin_unreachable();
}

/* Return the template instance of field_division corresponding to the given
 * FieldType. */
std::function<Field(const Field&, const Field&)> compile_division(
    Schema::FieldType type
) {
    switch (type) {
        case Schema::FieldType::INT: return field_division<int>;
        case Schema::FieldType::REAL: return field_division<double>;
        case Schema::FieldType::TEXT:
            throw FieldTypeException("INT/REAL", "TEXT");
    };
    __builtin_unreachable();
}

} // namespace

} // namespace minisql::planner