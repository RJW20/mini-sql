#include "planner/compiler.hpp"

#include <functional>

#include "row/field.hpp"
#include "row/schema.hpp"
#include "varchar.hpp"

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

} // namespace minisql::planner