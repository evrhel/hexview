#ifndef UTIL_H
#define UTIL_H

#include "defs.h"

#define KEY(val) ((key_t)(val))
#define VALUE(val) ((value_t)(val))
#define STRCMP ((compare_fn)&__string_compare_fn)
#define STRCPY ((copy_fn)&__string_copy_fn)
#define STRFREE ((copy_fn)&free)

typedef void *key_t;
typedef void *value_t;
typedef struct alist_s alist_t;

typedef int(*compare_fn)(void *first, void *second);
typedef void *(*copy_fn)(void *val);
typedef void(*free_fn)(void *val);

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

	void *addr;
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

// Returns whether two strings are case-insensitively equal.
// Parameters:
// - first: First string to compare. Must be non-NULL.
// - second: Second string to compare. Must be non-NULL.
//
// Returns:
// Nonzero if case-insensitively equal.
int equals_ignore_case(const char *first, const char *second);

// Create a new associative list.
// Parameters:
// - keycompare: Function used to compare two keys for equality. NULL
// compares by value.
// - keycopy: Function used to copy keys. NULL copies by value.
// - keyfree: Function used to free keys. NULL does nothing.
// - valuecopy: Function used to copy values. NULL copies by value.
// - valuefree: Function used to free values. NULL does nothing.
//
// Returns:
// A pointer to an alist_t on success and NULL on failure.
alist_t *alist_create(compare_fn keycompare, copy_fn keycopy, free_fn keyfree, copy_fn valuecopy, free_fn valuefree);

// Insert a key-value pairing into an associative list. If a pairing
// alreadys exists, it will be overwritten.
// Parameters:
// - alist: The list to insert into.
// - key: The key of the pairing. Cannot be NULL.
// - value: The value of the pairing.
void alist_insert(alist_t *alist, key_t key, value_t value);

// Finds a value by its pair and returns a pointer to the value.
// If the value is dereferenced, the value within the associative
// list can be modified directly. Ensure correct copying/freeing
// is used if custom copy/free functions were specified when
// calling alist_create.
// Parameters:
// - alist: The list to find the pairing in.
// - key: The key of the pairing.
//
// Returns:
// A pointer to the value associated with that key, or NULL if
// no such pairing exists.
value_t *alist_find(alist_t *alist, key_t key);

// Frees an alist_t allocated with alist_create.
// Parameters:
// - alist: The list to free, can be NULL.
void alist_free(alist_t *alist);

int __string_compare_fn(const char *first, const char *second);
char *__string_copy_fn(const char *value);

#endif