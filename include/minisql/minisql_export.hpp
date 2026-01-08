#ifndef MINISQL_EXPORT_HPP
#define MINISQL_EXPORT_HPP

#ifdef _WIN32
  #ifdef MINISQL_EXPORTS
    #define MINISQL_API __declspec(dllexport)
  #else
    #define MINISQL_API __declspec(dllimport)
  #endif
#else
  #define MINISQL_API
#endif

#endif // MINISQL_EXPORT_HPP