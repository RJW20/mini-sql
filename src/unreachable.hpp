#ifndef MINISQL_UNREACHABLE_HPP
#define MINISQL_UNREACHABLE_HPP

namespace minisql {

// Used to mark impossible code branches to the compiler.
[[noreturn]] inline void unreachable() {
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
}

} // namespace minisql

#endif // MINISQL_UNREACHABLE_HPP