#ifndef MINISQL_EXPORT_HPP
#define MINISQL_EXPORT_HPP

// For Windows (MSVC, MinGW, Cygwin)
#if defined(_WIN32) || defined(__CYGWIN__)
    // Static build
    #if defined(MINISQL_STATIC)
        #define MINISQL_API
    // Shared build
    #elif defined(MINISQL_EXPORTS)
        #define MINISQL_API __declspec(dllexport)
    // Using DLL
    #else
        #define MINISQL_API __declspec(dllimport)
    #endif
#else
    // Linux / macOS
    #define MINISQL_API
#endif

#endif // MINISQL_EXPORT_HPP