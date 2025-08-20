#include "planner/compiler.hpp"

#include <functional>

#include "field.hpp"
#include "varchar.hpp"
#include "exceptions.hpp"

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

} // namespace

} // namespace minisql::planner