#include "util.h"

#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <string.h>

#if _WIN32
#include <Windows.h>
#elif __linux__ || __APPLE__
#endif

struct alist_node
{
	akey_t key;
	avalue_t value;

	struct alist_node *next;
};

struct alist_s
{
	compare_fn keycompare;
	copy_fn keycopy;
	free_fn keyfree;
	copy_fn valuecopy;
	free_fn valuefree;

	struct alist_node *front;
};

// recursively free a node
static void alist_free_node(alist_t *alist, struct alist_node *node);

// find a node
static struct alist_node *alist_find_node(alist_t *alist, struct alist_node *node, akey_t key);

static int default_compare_fn(void *first, void *second);  // does first == second
static void *default_copy_fn(void *key);  // returns key
static void default_free_fn(void *key);  // does nothing

uint16
swap_endianess16(uint16 num)
{
	return (num >> 8) | (num << 8);
}

uint32
swap_endianess32(uint32 num)
{
	return	((num >> 24) & 0xff) |
			((num << 8) & 0xff0000) |
			((num >> 8) & 0xff00) |
			((num << 24) & 0xff000000);
}

uint64
swap_endianess64(uint64 num)
{
	num = ((num << 8) & 0xff00ff00ff00ff00) | ((num >> 8) & 0xff00ff00ff00ff);
	num = ((num << 16) & 0xffff0000ffff0000) | ((num >> 16) & 0xffff0000ffff);
	return (num << 32) | (num >> 32);
}

int
to_native_endianess(const value_u *in, int max_read, int endianess, outvalues_t *const out)
{
#if NATIVE_ENDIANESS == 0
	int toread;

	toread = max_read < sizeof(value_u) ? max_read : sizeof(value_u);

	memset(out, 0, sizeof(outvalues_t));
	out->char8 = (char8_t *)in;

	switch (endianess)
	{
	case LittleEndian:
		if (max_read < 1) break;
		out->ui8 = in->ui8;

		if (max_read < 2) break;
		out->ui16 = in->ui16;

		if (max_read < 4) break;
		out->ui32 = in->ui32;

		if (max_read < 8) break;
		out->ui64 = in->ui64;

		break;
	case BigEndian:
		if (max_read < 1) break;
		out->ui8 = in->ui8;

		if (max_read < 2) break;
		out->ui16 = swap_endianess16(in->ui16);

		if (max_read < 4)
		{
			max_read = 2;
			break;
		}
		out->ui32 = swap_endianess32(in->ui32);

		if (max_read < 8)
		{
			max_read = 4;
			break;
		}
		out->ui64 = swap_endianess64(in->ui64);

		break;
	}

	return max_read;
#elif NATIVE_ENDIANESS == 1
	// not supported
#endif
}

int
readline(char *const out, int maxcount)
{
	char c;
	int off;

	for (off = 0; off < maxcount; off++)
	{
		c = fgetc(stdin);
		if (c == '\n')
		{
			out[off] = 0;
			break;
		}
		out[off] = c;
	}

	return off;
}

int
equals_ignore_case(const char *first, const char *second)
{
	for (; *first && *second; first++, second++)
	{
		if (*first == *second) continue;

		if (*first >= 'A' && *first <= 'Z')
		{
			if (*first + 0x20 == *second)
				continue;
		}
		else if (*first >= 'a' && *second <= 'z')
		{
			if (*first - 0x20 == *second)
				continue;
		}

		return 0;
	}
	return *first == *second;
}

alist_t *
alist_create(compare_fn keycompare, copy_fn keycopy, free_fn keyfree, copy_fn valuecopy, free_fn valuefree)
{
	alist_t *alist;

	alist = malloc(sizeof(alist_t));
	if (!alist)
		return NULL;

	alist->keycompare = keycompare ? keycompare : &default_compare_fn;
	alist->keycopy = keycopy ? keycopy : &default_copy_fn;
	alist->keyfree = keyfree ? keyfree : &default_free_fn;
	alist->valuecopy = valuecopy ? valuecopy : &default_copy_fn;
	alist->valuefree = valuefree ? valuefree : &default_free_fn;

	alist->front = NULL;

	return alist;
}

void
alist_insert(alist_t *alist, akey_t key, avalue_t value)
{
	struct alist_node *node;
	avalue_t nval;

	if (key == NULL) return;

	node = alist_find_node(alist, alist->front, key);
	if (!node)
	{
		// not found
		node = malloc(sizeof(struct alist_node));
		if (!node) return;
		
		node->key = alist->keycopy(key);
		node->value = alist->valuecopy(value);

		node->next = alist->front;
		alist->front = node;
	}
	else
	{
		// replace value
		nval = alist->valuecopy(value);
		alist->valuefree(node->value);
		node->value = nval;
	}
}

avalue_t *
alist_find(alist_t *alist, akey_t key)
{
	struct alist_node *node;

	if (!key)
		return NULL;

	node = alist_find_node(alist, alist->front, key);
	return node ? &node->value : NULL;
}

void
alist_free(alist_t *alist)
{
	if (alist)
		alist_free_node(alist, alist->front);
}

void
set_console_color(int color)
{
#if _WIN32

#elif __linux__ || __APPLE__

#endif
}

void
set_console_style(int style)
{
#if _WIN32

#elif __linux__ || __APPLE__

#endif
}

int
__string_compare_fn(const char *first, const char *second)
{
	return !strcmp(first, second);
}

char *
__string_copy_fn(const char *value)
{
	char *result;
	int len;

	if (!value)
		return NULL;

	len = strlen(value);
	result = malloc(len + 1);
	if (!result)
		return NULL;
	result[len] = 0;

	memcpy(result, value, len);

	return result;
}

static void
alist_free_node(alist_t *alist, struct alist_node *node)
{
	assert(node->key != NULL);
	alist->keyfree(node->key);

	if (node->value) alist->valuefree(node->value);

	if (node->next)
		alist_free_node(alist, node->next);

	free(node);
}

static struct alist_node *
alist_find_node(alist_t *alist, struct alist_node *node, akey_t key)
{
	if (!node)
		return NULL;

	if (alist->keycompare(node->key, key))
		return node;
	return alist_find_node(alist, node->next, key);
}

static int
default_compare_fn(void *first, void *second)
{
	return first == second;
}

static void *
default_copy_fn(void *key)
{
	return key;
}

static void
default_free_fn(void *key)
{
	// do nothing
}