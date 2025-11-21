#ifndef MINISQL_INSTANTIATOR_HPP
#define MINISQL_INSTANTIATOR_HPP

#include <variant>

#include "minisql/field.hpp"

namespace minisql {

namespace {

template <template <typename> typename FuncWrapper, typename Variant>
struct InstantiateForVariant;

template <template <typename> typename FuncWrapper, typename... Ts>
struct InstantiateForVariant<FuncWrapper, std::variant<Ts...>> {
    static void instantiate() {
        (FuncWrapper<Ts>::instantiate(), ...);
    }
};

} // namespace

/* Struct for instantiating a templated function or method for all types in
 * Field.
 * Call InstantiateForField<FuncWrapper>::instantiate() where
 * FuncWrapper::instantiate calls the desired function with correct signature.
 */
template <template <typename> typename FuncWrapper>
using InstantiateForField = InstantiateForVariant<FuncWrapper, Field>;

} // namespace minisql

#endif // MINISQL_INSTANTIATOR_HPP