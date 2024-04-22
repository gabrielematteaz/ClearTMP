#include < Windows.h >
#include < string >
#include < iostream >

struct ClearDirResult
{
	DWORD delFiles;
	std::uint_least64_t delBytes;

	constexpr ClearDirResult& operator += (const ClearDirResult& other) noexcept
	{
		delFiles += other.delFiles;
		delBytes += other.delBytes;

		return *this;
	}
};

ClearDirResult ClearDir(LPCWSTR dirPath)
{
	ClearDirResult result{};

	/* Save the current directory path to restore it later */
	std::wstring curDirPath;
	curDirPath.reserve(GetCurrentDirectoryW(0, NULL));
	GetCurrentDirectoryW(static_cast < DWORD > (curDirPath.capacity()), curDirPath.data());

	/* Needed to delete files using "relative" path */
	SetCurrentDirectoryW(dirPath);

	/* Modify 'dirPath' so that 'FindFirstFileW' do not fail */
	std::wstring modifiedDirPath(dirPath);
	modifiedDirPath.append(L"\\*");

	WIN32_FIND_DATAW findData;
	HANDLE findHandle = FindFirstFileW(modifiedDirPath.c_str(), &findData);
	if (findHandle != NULL)
	{
		do
		{
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				/* Inner condition to prevent being treated as file */
				if (std::wcscmp(findData.cFileName, L".") != 0 && std::wcscmp(findData.cFileName, L"..") != 0)
				{
					result += ClearDir(findData.cFileName);
					RemoveDirectoryW(findData.cFileName);
				}
			}
			else if (DeleteFileW(findData.cFileName))
			{
				result.delFiles++;
				result.delBytes += (std::uint_least64_t)findData.nFileSizeHigh << 32 | findData.nFileSizeLow;
			}
		} while (FindNextFileW(findHandle, &findData));
		FindClose(findHandle);
	}

	/* Restore previous directory path */
	SetCurrentDirectoryW(curDirPath.c_str());

	return result;
}

int main()
{
	try
	{
		/* Get the temporary files directory path */
		std::wstring tempFilesDirPath;
		tempFilesDirPath.reserve(GetTempPath2W(0, NULL));
		GetTempPath2W(static_cast < DWORD > (tempFilesDirPath.capacity()), tempFilesDirPath.data());

		auto result = ClearDir(tempFilesDirPath.c_str());

		std::cout << "Cleared " << result.delFiles << " files (" << result.delBytes << " bytes)" << std::endl;
		std::cin.get();
	}
	catch (const std::bad_alloc&)
	{
		std::cout << "An allocation error occurred" << std::endl;
	}

	return 0;
}