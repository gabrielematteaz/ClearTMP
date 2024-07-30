#include "Filesystem.h"

namespace ClearTMP
{
	using wstring = std::basic_string < wchar_t, std::char_traits < wchar_t >, Matteaz::Allocator < wchar_t > >;

	void ClearFiles(wstring &directory)
	{
		auto length = directory.length();

		for (Matteaz::DirectoryIterator iterator(directory.c_str(), directory.get_allocator()); iterator; ++iterator) {
			auto entry = *iterator;

			if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (entry.cFileName[0] == L'.' && (entry.cFileName[1] == L'\0' || entry.cFileName[1] == L'.' && entry.cFileName[2] == L'\0')) continue;

				directory.replace(length, directory.length(), L"\\");
				directory.append(entry.cFileName);
				ClearFiles(directory);
			}
			else {
				directory.replace(length, directory.length(), L"\\");
				directory.append(entry.cFileName);
				DeleteFileW(directory.c_str());
			}
		}

		directory.resize(length);
		RemoveDirectoryW(directory.c_str());
	}
}

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
	HRESULT code = S_OK;

	try {
		ClearTMP::wstring directory(MAX_PATH + 1, L'\0');
		SIZE_T result = GetTempPath2W(static_cast < DWORD > (directory.capacity()), directory.data());

		if (result == 0) throw Matteaz::SystemError(L"GetTempPath2W failed");

		directory.resize(result - 1);
		ClearTMP::ClearFiles(directory);
	}
	catch (const Matteaz::SystemError &error) {
		code = HRESULT_FROM_WIN32(error.code);
	}
	catch (const Matteaz::Exception &) {
		code = E_FAIL;
	}

	return code;
}
