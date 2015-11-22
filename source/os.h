#ifndef OS_H_INCLUDED
#define OS_H_INCLUDED


// EXPORT / IMPORT  definitions for several compilers
//  Microsoft 
#if defined(_MSC_VER)
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
//  GCC
#elif defined(_GCC)
#define EXPORT __attribute__((visibility("default")))
#define IMPORT
// no idea which compiler is used
#else
#define EXPORT
#define IMPORT
#pragma warning Unknown dynamic link import/export semantics.
#endif

// FUNCTION NAME definitions for several compilers
// windows uses __FUNCTION__, others take __func__, so override for windows!
#ifndef _MSC_VER
#define __FUNCTION__ __func__
#endif


#endif // OS_H_INCLUDED