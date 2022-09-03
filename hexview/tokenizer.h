#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef struct token_list_s token_list_t;
struct token_list_s
{
	struct
	{
		char *string;
		int integer;
	} token;
	token_list_t *next, *prev;
};

// Tokenize a string into a list of tokens
token_list_t *tokenize(const char *string);

// Returns the first token in a token list.
token_list_t *front_token(token_list_t *node);

// Seeks into a token list by a positive or negative offset.
token_list_t *offset_token(token_list_t *node, int offset);

// Frees a token list.
void free_token_list(token_list_t *list);

#endif