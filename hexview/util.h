#ifndef UTIL_H
#define UTIL_H

#include "defs.h"

typedef union value_u value_u;
union value_u
{
	int8 i8;
	uint8 ui8;
	int16 i16;
	uint16 ui16;
	int32 i32;
	uint32 ui32;
	int64 i64;
	uint64 ui64;

	float32 f32;
	float64 f64;
};

typedef struct outvalues_s outvalues_t;
struct outvalues_s
{
	union
	{
		int8 i8;
		uint8 ui8;
	};

	union
	{
		int16 i16;
		uint16 ui16;
	};

	union
	{
		int32 i32;
		uint32 ui32;
		float32 f32;
	};

	union
	{
		int64 i64;
		uint64 ui64;
		float64 f64;
	};

	union
	{
		utf8_t *utf8;
		utf16_t *utf16;
	};
};

inline uint16
swap_endianess16(uint16 num) { return (num >> 8) | (num << 8); }

inline uint32
swap_endianess32(uint32 num)
{
	return	((num >> 24) & 0xff) |
			((num << 8) & 0xff0000) |
			((num >> 8) & 0xff00) |
			((num << 24) & 0xff000000);
}

inline uint64
swap_endianess64(uint64 num)
{
	num = ((num << 8) & 0xff00ff00ff00ff00) | ((num >> 8) & 0xff00ff00ff00ff);
	num = ((num << 16) & 0xffff0000ffff0000) | ((num >> 16) & 0xffff0000ffff);
	return (num << 32) | (num >> 32);
}

// Convert data to the system's native endianess.
// Parameters:
// - in: Values to convert.
// - max_read: Maximum number of bytes which can be read from in.
// - endianess: The endianess of the input data.
// - out: Output parameter which will contain the converted data.
//
// Returns:
// Number of bytes written to out.
int to_native_endianess(const value_u *in, int max_read, int endianess, outvalues_t *const out);

// Read a line from stdin.
// Parameters:
// - out: Destination string.
// - maxcount: Maximum number of characters to read from stdin.
//
// Returns:
// The length of the string stored in out on return.
int readline(char *const out, int maxcount);

#endif