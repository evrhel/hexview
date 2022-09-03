#include "file.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include <Windows.h>
#endif

typedef struct win32_file_s win32_file_t;
struct win32_file_s
{
	HANDLE hFile;
	HANDLE hMap;
};

file_t *
open_file(const char *filename)
{
	file_t *result;

#ifdef WIN32
	DWORD dwSize, dwHigh;
	win32_file_t *file32;

	result = malloc(offsetof(file_t, reserved) + sizeof(win32_file_t));
	if (!result)
		return NULL;
	file32 = &result->reserved;

	file32->hFile = CreateFileA(
		filename,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (!file32->hFile || file32->hFile == INVALID_HANDLE_VALUE)
	{
		free(result);
		return NULL;
	}

	dwSize = GetFileSize(file32->hFile, &dwHigh);
	result->size = (int)dwSize;

	file32->hMap = CreateFileMappingA(
		file32->hFile,
		NULL,
		PAGE_READONLY,
		0,
		dwSize,
		NULL
	);

	if (!file32->hMap)
	{
		CloseHandle(file32->hFile);
		free(result);
		return NULL;
	}

	result->data = MapViewOfFile(
		file32->hMap,
		FILE_MAP_READ,
		0,
		0,
		dwSize
	);

	if (!result->data)
	{
		CloseHandle(file32->hMap);
		CloseHandle(file32->hFile);
		free(result);
		return NULL;
	}
#else

#endif

	return result;
}

void
close_file(file_t *file)
{
#ifdef WIN32
	win32_file_t *file32;

	file32 = &file->reserved;

	UnmapViewOfFile(file->data);
	CloseHandle(file32->hMap);
	CloseHandle(file32->hFile);

	free(file);
#else

#endif
}