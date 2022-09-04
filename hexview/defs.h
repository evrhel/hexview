#ifndef DEFS_H
#define DEFS_H

#include <stdlib.h>

enum
{
	LittleEndian = 0,
	BigEndian = 1
};

#define NATIVE_ENDIANESS 0

#ifdef _WIN32
typedef unsigned __int8 byte;

typedef char utf8_t;
typedef wchar_t utf16_t;

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

#endif

enum
{
	Int8, Uint8,
	Int16, Uint16,
	Int32, Uint32,
	Int64, Uint64,
	Float32, Float64,
	Utf8, Utf16
};

#endif