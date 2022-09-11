#ifndef DEFS_H
#define DEFS_H

#include <stdlib.h>
#include <wchar.h>

enum
{
	LittleEndian = 0,
	BigEndian = 1
};

#define NATIVE_ENDIANESS 0

#ifdef _WIN32
typedef unsigned __int8 byte;

typedef char char8_t;
#ifdef _WCHAR_T_DEFINED
typedef wchar_t char16_t;
#else
typedef unsigned short char16_t;
#endif

typedef __int8 int8;
typedef unsigned __int8 uint8;
typedef __int16 int16;
typedef unsigned __int16 uint16;
typedef __int32 int32;
typedef unsigned __int32 uint32;
typedef __int64 int64;
typedef unsigned __int64 uint64;
typedef float float32;
typedef double float64;

#else

typedef unsigned char byte;

typedef char char8_t;

#if __WCHAR_MAX__ > 0x10000
typedef short unsigned int char16_t;
#else
typedef wchar_t char16_t;
#endif

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;
typedef float float32;
typedef double float64;

#endif

enum
{
	Int8, Uint8,
	Int16, Uint16,
	Int32, Uint32,
	Int64, Uint64,
	Float32, Float64,
	Char8, Char16
};

#endif