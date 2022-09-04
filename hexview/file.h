#ifndef FILE_H
#define FILE_H

#include "defs.h"

typedef struct file_s file_t;
struct file_s
{
	byte *data;  // Pointer to the start of the file's data. Addressing valid from [data, data + size).
	int size;    // Size of the file.

	byte reserved[1];
};

// Open a file for reading. The file will be memory mapped if its size is greater than the page size.
// Parameters:
// - filename: The name of the file to open.
//
// Returns:
// The opened file, or NULL if it could not be opened.
file_t *open_file(const char *filename);

// Close an open file.
// Parameters:
// - file: The file to close.
void close_file(file_t *file);

#endif