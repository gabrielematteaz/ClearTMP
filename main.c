#define _CRT_SECURE_NO_WARNINGS
#include < assert.h >
#include < Windows.h >

struct wstr_t
{
	wchar_t* buffer;
	size_t size;
};

int wstr_alloc(struct wstr_t* const wstr, size_t size)
{
	assert(wstr != NULL);
	if (size == 0) return 0;
	wstr->buffer = malloc(size * sizeof(wchar_t));
	if (wstr->buffer == NULL)
	{
		wstr->size = 0;
		return 0;
	}
	wstr->size = size;
	return 1;
}

int wstr_realloc(struct wstr_t* const wstr, size_t size)
{
	assert(wstr != NULL);
	if (size == 0) return 0;
	wchar_t* buffer = realloc(wstr->buffer, size * sizeof(wchar_t));
	if (buffer == NULL) return 0;
	wstr->buffer = buffer;
	wstr->size = size;
	return 1;
}

int wstr_overwrite(struct wstr_t* const wstr, size_t offset, const wchar_t* what)
{
	assert(wstr != NULL);
	assert(what != NULL);
	size_t what_size = wcslen(what) + 1;
	size_t size = offset + what_size;
	if (size > wstr->size && !wstr_realloc(wstr, size)) return 0;
	wcscpy(wstr->buffer + offset, what);
	return 1;
}

void wstr_free(struct wstr_t* const wstr)
{
	assert(wstr != NULL);
	free(wstr->buffer);
	wstr->buffer = NULL;
	wstr->size = 0;
}

void clear_dir(struct wstr_t* const path, size_t len)
{
	assert(path != NULL);
	if (!wstr_overwrite(path, len, L"\\*")) return;
	WIN32_FIND_DATAW* find_data = malloc(sizeof(WIN32_FIND_DATAW));
	if (find_data == NULL) return;
	HANDLE find_handle = FindFirstFileW(path->buffer, find_data);
	if (find_handle != INVALID_HANDLE_VALUE)
	{
		len++;
		do
		{
			if (find_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (find_data->cFileName[0] == L'.' && (find_data->cFileName[1] == L'\0' || (find_data->cFileName[1] == L'.' && find_data->cFileName[2] == L'\0'))) continue;
				if (!wstr_overwrite(path, len, find_data->cFileName)) break;
				size_t new_len = len + wcslen(find_data->cFileName);
				clear_dir(path, new_len);
				path->buffer[new_len] = L'\0';
				RemoveDirectoryW(path->buffer);
			}
			else
			{
				if (!wstr_overwrite(path, len, find_data->cFileName)) break;
				DeleteFileW(path->buffer);
			}
		} while (FindNextFileW(find_handle, find_data) != 0);
	}
	FindClose(find_handle);
	free(find_data);
}

int main(void)
{
	struct wstr_t path;
	if (!wstr_alloc(&path, GetTempPath2W(0, NULL))) return 1;
	clear_dir(&path, (size_t)GetTempPath2W((DWORD)path.size, path.buffer) - 1);
	wstr_free(&path);
	return 0;
}