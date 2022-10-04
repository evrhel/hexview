#include "pattern.h"

#include <string.h>

#include "tokenizer.h"
#include "util.h"

#define INITIAL_PATTERN_CAP 64

struct pattern_s
{
	short *bytes;
	unsigned int count;
	unsigned int capacity;
};

static int has_prefix(const char *s, const char *prefix);
static void append_byte(pattern_t *pattern, unsigned short b);
static void append_memory(pattern_t *pattern, void *mem, unsigned int length);
static void append_bytes(pattern_t *pattern, short *bytes, unsigned int count);

pattern_t *
pattern_generate(token_list_t *tokens, unsigned int *const size)
{
	pattern_t *pattern;
	token_list_t *it;
	char *s, *end;
	value_u vals;
	size_t len;

	pattern = malloc(sizeof(pattern_t));
	if (!pattern)
		return NULL;

	pattern->count = 0;
	pattern->capacity = INITIAL_PATTERN_CAP;

	pattern->bytes = malloc(pattern->capacity * sizeof(short));
	if (!pattern->bytes)
	{
		free(pattern);
		return NULL;
	}

	it = tokens;
	while (it)
	{
		s = it->token.string;
		if (has_prefix(s, "?"))
		{
			s++;
			vals.ui64 = 0;
			if (*s)
				vals.ui64 = strtoull(s, &end, 0);
			for (; vals.ui64 != 0; vals.ui64--) append_byte(pattern, 0xff00);
		}
		else if (has_prefix(s, "i8"))
		{
			s += 2;
			if (!*s) goto on_error;
			vals.i8 = strtoll(s, &end, 0);
			append_memory(pattern, &vals, sizeof(int8));
		}
		else if (has_prefix(s, "ui8"))
		{
			s += 3;
			if (!*s) goto on_error;
			vals.ui8 = strtoull(s, &end, 0);
			append_memory(pattern, &vals, sizeof(uint8));
		}
		else if (has_prefix(s, "i16"))
		{
			s += 3;
			if (!*s) goto on_error;
			vals.i16 = strtoll(s, &end, 0);
			append_memory(pattern, &vals, sizeof(int16));
		}
		else if (has_prefix(s, "ui16"))
		{
			s += 4;
			if (!*s) goto on_error;
			vals.ui16 = strtoll(s, &end, 0);
			append_memory(pattern, &vals, sizeof(uint16));
		}
		else if (has_prefix(s, "i32"))
		{
			s += 3;
			if (!*s) goto on_error;
			vals.i32 = strtoll(s, &end, 0);
			append_memory(pattern, &vals, sizeof(int32));
		}
		else if (has_prefix(s, "ui32"))
		{
			s += 4;
			if (!*s) goto on_error;
			vals.ui32 = strtoll(s, &end, 0);
			append_memory(pattern, &vals, sizeof(int32));
		}
		else if (has_prefix(s, "i64"))
		{
			s += 3;
			if (!*s) goto on_error;
			vals.i64 = strtoll(s, &end, 0);
			append_memory(pattern, &vals, sizeof(int64));
		}
		else if (has_prefix(s, "ui64"))
		{
			s += 4;
			if (!*s) goto on_error;
			vals.ui64 = strtoll(s, &end, 0);
			append_memory(pattern, &vals, sizeof(uint64));
		}
		else if (has_prefix(s, "f32"))
		{
			s += 3;
			if (!*s) goto on_error;
			vals.f32 = (float32)atof(s);
			append_memory(pattern, &vals, sizeof(float32));
		}
		else if (has_prefix(s, "f64"))
		{
			s += 3;
			if (!*s) goto on_error;
			vals.f64 = atof(s);
			append_memory(pattern, &vals, sizeof(float64));
		}
		else if (has_prefix(s, "c"))
		{
			s++;
			if (!*s) goto on_error;
			append_byte(pattern, *((uint8 *)s));
		}
		else if (has_prefix(s, "wc"))
		{
			s += 2;
			if (!*s) goto on_error;
			vals.ui16 = *((uint8 *)s);
			append_memory(pattern, &vals, sizeof(uint16));
		}
		else if (has_prefix(s, "s"))
		{
			s++;
			if (!*s) goto on_error;
			append_memory(pattern, s, strlen(s));
		}
		else if (has_prefix(s, "sn"))
		{
			s += 2;
			if (!*s) goto on_error;
			append_memory(pattern, s, strlen(s) + 1);
		}
		else if (has_prefix(s, "ws"))
		{
			s += 2;
			if (!*s) goto on_error;
			len = strlen(s);
			for (; len != 0; len--, s++)
			{
				vals.ui16 = *s;
				append_memory(pattern, &vals, sizeof(uint16));
			}
		}
		else if (has_prefix(s, "wsn"))
		{
			s += 3;
			if (!*s) goto on_error;
			len = strlen(s) + 1;
			for (; len != 0; len--, s++)
			{
				vals.ui16 = *s;
				append_memory(pattern, &vals, sizeof(uint16));
			}
		}
		else
		{
			vals.ui8 = strtoull(it->token.string, &end, 16);
			append_byte(pattern, vals.ui8);
		}

		it = it->next;
	}

	*size = pattern->count;
	return pattern;

on_error:
	pattern_free(pattern);
	return NULL;
}

void
pattern_free(pattern_t *pattern)
{
	if (!pattern) return;
	free(pattern->bytes);
	free(pattern);
}

int
pattern_find_next(pattern_t *pattern, const byte *bytes, unsigned int maxsearch, unsigned int *const out)
{
	byte *match, *test, *next;
	short *tomatch;
	unsigned int remaining;
	short m;
	byte b;
	const short mask = 0xff00;

	test = bytes; next = test + 1;
	tomatch = pattern->bytes;
	remaining = pattern->count;

	while (remaining <= maxsearch && remaining != 0)
	{
		m = *tomatch;
		b = *test;

		if ((m & mask) && (byte)m != b)
		{
			// shift bytes to test
			test = next; next++;
			maxsearch--;

			// reset match array
			tomatch = pattern->bytes;
			remaining = pattern->count;

			continue;
		}

		test++;
		tomatch++;
		remaining--;
	}

	if (remaining != 0)
		return 0;
	*out = (next - 1) - bytes;
	return 1;
}

static int
has_prefix(const char *s, const char *prefix)
{
	while (*s && *prefix)
	{
		if (*s != *prefix)
			return 0;
		s++;
		prefix++;
	}

	// prefix is longer than s
	if (!*s && *prefix)
		return 0;

	return !*prefix;
}

static void
append_byte(pattern_t *pattern, unsigned short b)
{
	uint32 ncap;
	uint16 *nbuf;

	if (pattern->count == pattern->capacity)
	{
		ncap = pattern->capacity << 1;
		nbuf = realloc(pattern->bytes, ncap * sizeof(short));
		if (!nbuf)
			return;
		pattern->capacity = ncap;
		pattern->bytes = nbuf;
	}

	pattern->bytes[pattern->count++] = (short)b;
}

static void
append_memory(pattern_t *pattern, void *mem, unsigned int length)
{
	byte *b = (byte *)mem;
	for (; length != 0; length--, b++)
		append_byte(pattern, *b);
}

static void
append_bytes(pattern_t *pattern, short *bytes, unsigned int count)
{

}