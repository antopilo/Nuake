#ifndef PLATFORM_H
#define PLATFORM_H

#if defined(_MSC_VER)
    //  Microsoft 
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    //  GCC
    #define EXPORT __attribute__((visibility("default")))
    #define IMPORT
#else
    //  do nothing and hope for the best?
    #define EXPORT
    #define IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif

#if defined(_MSC_VER)
    #define STRDUP(s) _strdup(s)
#else
    #define STRDUP(s) strdup(s)
#endif

#endif