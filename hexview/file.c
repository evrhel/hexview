#include "file.h"

#include <stdlib.h>
#include <stdio.h>

#if _WIN32
#include <Windows.h>

struct win32_file
{
	HANDLE hFile;  // file handle
	HANDLE hMap;   // file mapping
};

#elif __linux__ || __APPLE__

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>

struct linux_file
{
	int file;  // file descriptor
};

#endif

file_t *
open_file(const char *filename)
{
	file_t *result;

#if _WIN32
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

		CloseHandle(file32->hFile);
		file32->hFile = NULL;
	}
#elif __linux__ || __APPLE__
	struct linux_file *linux_file;
	struct stat st;
	int pagesize;
	ssize_t remaining;
	ssize_t bytes_read;

	result = malloc(offsetof(file_t, reserved) + sizeof(struct linux_file));
	linux_file = (struct linux_file *)&result->reserved;

	linux_file->file = open(filename, O_RDONLY);
	if (!linux_file->file)
	{
		free(result);
		return NULL;
	}

	pagesize = getpagesize();

	fstat(linux_file->file, &st);
	result->size = st.st_size;
	if (result->size == 0)
	{
		close(linux_file->file);
		free(result);
		return NULL;
	}

	if (result->size >= pagesize)
	{
		result->data = mmap(NULL, result->size, PROT_READ, MAP_SHARED, linux_file->file, 0);
		if (result->data == MAP_FAILED)
		{
			close(linux_file->file);
			free(result);
			return NULL;
		}
	}
	else
	{
		result->data = malloc(result->size);
		if (!result->data)
		{
			close(linux_file->file);
			free(result);
			return NULL;
		}

		remaining = result->size;
		do
		{
			bytes_read = read(linux_file->file, result->data, remaining);
			if (bytes_read == 0)
				break;
			else if (bytes_read == -1)
			{
				if (errno == EAGAIN)
					continue;

				close(linux_file->file);
				free(result);
				return NULL;
			}
			remaining -= bytes_read;
		} while (remaining > 0);

		close(linux_file->file);
		linux_file->file = 0;
	}
#endif

	return result;
}

void
close_file(file_t *file)
{
#if _WIN32
	struct win32_file *file32;

	file32 = (struct win32_file *)&file->reserved;

	if (file32->hMap)
	{
		UnmapViewOfFile(file->data);
		CloseHandle(file32->hMap);
	}
	else
		free(file->data);

	if (file32->hFile)
		CloseHandle(file32->hFile);

	free(file);
#elif __linux__ || __APPLE__
	struct linux_file *linux_file;

	linux_file = (struct linux_file *)&file->reserved;

	if (linux_file->file)
	{
		munmap(file->data, file->size);
		close(linux_file->file);
	}
	else
		free(file->data);

	free(file);
#endif
}