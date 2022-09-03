#ifndef FILE_H
#define FILE_H

#include "defs.h"

typedef struct file_s file_t;
struct file_s
{
	byte *data;
	int size;

	byte reserved[1];
};

// open file for reading
file_t *open_file(const char *filename);

// close file
void close_file(file_t *file);

#endif