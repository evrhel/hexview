#ifndef PATTERN_H
#define PATTERN_H

#include "defs.h"

typedef struct pattern_s pattern_t;
typedef struct token_list_s token_list_t;

// Generates a pattern from a list of tokens.
//
// Parameters:
// - tokens: The list to generate the pattern from.
// - size: Output parameter giving the number of bytes the pattern
//         will match.
// 
// Returns:
// The pattern, or NULL if the pattern could not be parsed.
pattern_t *pattern_generate(token_list_t *tokens, unsigned int *const size);

// Frees a pattern.
//
// Parameters:
// - pattern: The pattern to free.
void pattern_free(pattern_t *pattern);

// Finds the next match of a pattern on an array of bytes.
//
// Parameters:
// - pattern: The pattern to test against.
// - bytes: Pointer to the start of the data to search.
// - maxsearch: The maximum number of bytes to search, must be
//              <= to the number of bytes bytes points to.
// - out: Output parameter to give the offset from bytes in which
//        a match was found,
//
//
// Returns:
// Nonzero if a match was found.
int pattern_find_next(pattern_t *pattern, const byte *bytes, unsigned int maxsearch, unsigned int *const out);

#endif
