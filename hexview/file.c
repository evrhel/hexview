#include "file.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>

struct win32_file
{
	HANDLE hFile;  // file handle
	HANDLE hMap;   // file mapping
};

#endif

file_t *
open_file(const char *filename)
{
	file_t *result;

#ifdef _WIN32
	DWORD dwSize, dwHigh;
	struct win32_file *file32;
	SYSTEM_INFO sysinfo;
	DWORD dwBytesRead;
	DWORD dwBytesRemaining;
	BOOL bResult;
	DWORD dwError;

	result = malloc(offsetof(file_t, reserved) + sizeof(struct win32_file));
	if (!result)
		return NULL;
	file32 = (struct win32_file *)&result->reserved;

	file32->hFile = CreateFileA(
		filename,
		GENERIC_READ,
		FILE_SHARE_READ,
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
	if (!result->size)
	{
		CloseHandle(file32->hFile);
		free(result);
		return NULL;
	}

	GetSystemInfo(&sysinfo);
	if (dwSize >= sysinfo.dwPageSize)
	{
		// create file mapping
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

		// create view
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
	}
	else
	{
		file32->hMap = NULL;
		result->data = malloc(dwSize);
		if (!result->data)
		{
			CloseHandle(file32->hFile);
			free(result);
			return NULL;
		}

		dwBytesRemaining = dwSize;
		do
		{
			bResult = ReadFile(file32->hFile, result->data, dwBytesRemaining, &dwBytesRead, NULL);
			if (!bResult)
			{
				dwError = GetLastError();
				if (dwError == ERROR_IO_PENDING)
					continue;

				free(result->data);
				CloseHandle(file32->hFile);
				free(result);
				return NULL;
			}

			dwBytesRemaining -= dwBytesRead;
		} while (dwBytesRemaining != 0);
	}
#else

#endif

	return result;
}

void
close_file(file_t *file)
{
#ifdef _WIN32
	struct win32_file *file32;

	file32 = (struct win32_file *)&file->reserved;

	if (file32->hMap)
	{
		UnmapViewOfFile(file->data);
		CloseHandle(file32->hMap);
	}

	CloseHandle(file32->hFile);

	free(file);
#else

#endif
}