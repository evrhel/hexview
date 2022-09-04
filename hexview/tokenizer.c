#include "tokenizer.h"

#include <string.h>
#include <stdlib.h>

// Used to build a null-terminated string
struct string_builder
{
	char *string;  // String being built
	int len;       // Length of current string
	int cap;       // Number of allocated bytes string points to
};

static token_list_t *
append_list(token_list_t *back);

static void
free_token_list_impl(token_list_t *front);

// Create a string builder
static struct string_builder *
builder_create();

// Free a string builder
static void
builder_free(struct string_builder *builder);

// Append a character to a string builder
static int
builder_append_char(struct string_builder *builder, char append);

// Append a string to a string builder
static int
builder_append(struct string_builder *builder, const char *append);

// Store the string in a string builder into a heap allocaed string
static char *
builder_store(struct string_builder *builder);

token_list_t *
tokenize(const char *string)
{
	const char *cursor;
	struct string_builder *builder;
	token_list_t *front, *back;
	char *end;
	int escape = 0;
	char c;
	char quote = 0;

	builder = builder_create();
	if (!builder)
		return NULL;

	back = front = NULL;

	for (cursor = string; c = *cursor; cursor++)
	{
		switch (c)
		{
		case ' ':
		case '\t':
			if (quote)
			{
				builder_append_char(builder, c);
				continue;
			}

			back = append_list(back);
			if (!back)
			{
				builder_free(builder);
				free_token_list(front);
				return NULL;
			}

			if (!front)
				front = back;

			back->token.string = builder_store(builder);
			back->token.integer = strtol(back->token.string, &end, 0);

			builder_free(builder);
			builder = builder_create();
			if (!builder)
			{
				free_token_list(front);
				return NULL;
			}
			break;
		case '"':
			if (quote == '\'')
			{
				builder_append_char(builder, c);
				continue;
			}

			if (escape)
			{
				builder_append_char(builder, c);
				escape = 0;
				break;
			}

			quote = quote ? 0 : '"';
			break;
		case '\'':
			if (quote == '"')
			{
				builder_append_char(builder, c);
				continue;
			}

			if (escape)
			{
				builder_append_char(builder, c);
				escape = 0;
				break;
			}
			

			quote = quote ? 0 : '\'';
			break;
		case '\\':
			if (escape)
				builder_append_char(builder, c);
			escape = !escape;
			break;
		default:
			if (!escape)
				builder_append_char(builder, c);
			escape = 0;
			break;
		}
	}

	if (builder->len > 0)
	{
		back = append_list(back);
		if (!back)
		{
			builder_free(builder);
			free_token_list(front);
			return NULL;
		}

		if (!front)
			front = back;

		back->token.string = builder_store(builder);
		if (!back->token.string)
		{
			builder_free(builder);
			free_token_list(front);
			return NULL;
		}

		back->token.integer = strtol(back->token.string, &end, 0);
	}

	builder_free(builder);

	return front;
}

token_list_t *
front_token(token_list_t *node)
{
	if (!node) return NULL;
	while (node->prev) node = node->prev;
	return node;
}

token_list_t *
offset_token(token_list_t *node, int offset)
{
	int i;

	if (offset > 0)
	{
		for (i = 0; i < offset && node;)
		{
			node = node->next;
			i++;
			if (i == offset)
				break;
		}
	}
	else
	{
		offset = -offset;
		for (i = 0; i < offset && node;)
		{
			node = node->prev;
			i++;
			if (i == offset)
				break;
		}
	}

	return node;
}

void
free_token_list(token_list_t *list)
{
	if (!list) return;
	list = front_token(list);
	free_token_list_impl(list);
}

static token_list_t *
append_list(token_list_t *back)
{
	token_list_t *result;

	result = calloc(1, sizeof(token_list_t));
	if (!result)
		return NULL;

	result->prev = back;
	if (back)
		back->next = result;

	return result;
}

static void
free_token_list_impl(token_list_t *front)
{
	if (front->next)
		free_token_list_impl(front->next);

	free(front->token.string);
	free(front);
}

static struct string_builder *
builder_create()
{
	struct string_builder *result;
	const int init_cap = 16;

	result = malloc(sizeof(struct string_builder));
	if (!result)
		return NULL;

	result->string = malloc(init_cap);
	if (!result->string)
	{
		free(result);
		return NULL;
	}

	result->len = 0;
	result->cap = init_cap;

	return result;
}

static void
builder_free(struct string_builder *builder)
{
	if (!builder) return;

	free(builder->string);
	free(builder);
}

static int
builder_append_char(struct string_builder *builder, char append)
{
	int nlen;
	int ncap;
	char *nbuf;

	nlen = builder->len + 1;
	if (nlen > builder->cap)
	{
		ncap = builder->cap * 2;
		nbuf = realloc(builder->string, nlen);
		if (!nbuf)
			return 0;
		builder->string = nbuf;
		builder->cap = ncap;
		return builder_append_char(builder, append);
	}

	*(builder->string + builder->len) = append;
	builder->len = nlen;
	return 1;
}

static int
builder_append(struct string_builder *builder, const char *append)
{
	for (; *append; append++)
	{
		if (!builder_append_char(builder, *append))
			return 0;
	}
	return 1;
}

static char *
builder_store(struct string_builder *builder)
{
	char *result;
	int size;

	size = builder->len + 1;
	result = malloc(size);
	if (!result)
		return NULL;

	memcpy(result, builder->string, size);
	result[builder->len] = 0;
	return result;
}