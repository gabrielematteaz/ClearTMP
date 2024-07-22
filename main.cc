#include < iostream >
#include "filesystem.h"

namespace clear_TMP
{
	using wstring = std::basic_string < wchar_t, std::char_traits < wchar_t >, matteaz::allocator < wchar_t > >;

	matteaz::allocator < std::byte > allocator;

	void clear_files(wstring &directory)
	{
		auto length = directory.length();

		for (matteaz::directory_iterator iterator(directory.c_str(), allocator); iterator; ++iterator) {
			auto entry = *iterator;

			if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (entry.cFileName[0] == L'.' && (entry.cFileName[1] == L'\0' || entry.cFileName[1] == L'.' && entry.cFileName[2] == L'\0')) continue;

				directory.replace(length, directory.length(), L"\\");
				directory.append(entry.cFileName);
				clear_files(directory);
				RemoveDirectoryW(directory.c_str());
			}
			else {
				directory.replace(length, directory.length(), L"\\");
				directory.append(entry.cFileName);
				DeleteFileW(directory.c_str());
			}
		}
	}
}

int WINAPI wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previous_instance, _In_ LPWSTR command_line, _In_ int show_command)
{
	HRESULT code = S_OK;

	try {
		auto size = GetTempPath2W(0, nullptr);
		clear_TMP::wstring directory(size, L'\0', clear_TMP::allocator);
		auto length = GetTempPath2W(static_cast < DWORD > (directory.capacity()), directory.data());

		if (length == 0) throw matteaz::HRESULT_error(HRESULT_FROM_WIN32(GetLastError()), L"GetTempPath2W failed");

		--length;
		directory.resize(length);
		clear_TMP::clear_files(directory);
	}
	catch (const matteaz::HRESULT_error &error) {
		code = error.code_;
	}
	catch (const matteaz::exception&) {
		code = E_FAIL;
	}

	return code;
}